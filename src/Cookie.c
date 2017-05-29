#include "Header.h"

#define SESSION_COOKIE_NAME "SessionToken"

void setCookie(struct http_request *req, char *name, char *value) {
    setCookieFull(req, name, value, NULL);
}

void setCookieFull(struct http_request *req, char *name, char *value, char *expiryDate) {
    struct kore_buf *buffer;

    char *initialValue;
    if (expiryDate != NULL) {
        initialValue = "$name$=$value$; HttpOnly; Secure; SameSite=strict; expires=$expiry$;";
    } else {
        initialValue = "$name$=$value$; HttpOnly; Secure; SameSite=strict;";
    }

    buffer = kore_buf_alloc(strlen(initialValue));
    kore_buf_append(buffer, initialValue, strlen(initialValue));

    kore_buf_replace_string(buffer, "$name$", name, strlen(name));
    kore_buf_replace_string(buffer, "$value$", value, strlen(value));
    if (expiryDate != NULL) {
        kore_buf_replace_string(buffer, "$expiry$", expiryDate, strlen(expiryDate));
    }
    http_response_header(req, "set-cookie", buffer->data);
}

void removeCookie(struct http_request *req, char *name) {
    setCookieFull(req, name, "Expired", "Thu, 01 Jan 1970 00:00:00 GMT");
}

void removeSessionCookie(struct http_request *req) {
    removeCookie(req, SESSION_COOKIE_NAME);
}


//TODO: should be decrypted
struct kore_buf *getCookieValue(struct http_request *req, char *name) {
    char *value;
    struct kore_buf *buffer;
    if (http_request_cookie(req, name, &value)) {
        buffer = kore_buf_alloc(STRING_SIZE);
        kore_buf_append(buffer, value, STRING_SIZE);
        return buffer;
    } else {
        return NULL;
    }
}

struct kore_buf *getSessionCookieValue(struct http_request *req) {
    return getCookieValue(req, SESSION_COOKIE_NAME);
}

// Creates Session Cookie
void createSessionCookie(struct http_request *req, int user_id) {
    char key[STRING_SIZE];
    calloc(key, sizeof(key));
    generateSessionKey(key);

    createSessionRow(user_id, key);
    setCookie(req, SESSION_COOKIE_NAME, key);
    getUserIdWithSession(key);
}


void generateSessionKey(char key[STRING_SIZE]) {
    char keySeed[STRING_SIZE];
    calloc(keySeed, sizeof(keySeed));
    const char keySet[] = {"123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$^&*()-_+,<.>?`~"};
    generate_random(keySeed, STRING_SIZE);
    for (int i = 0; i < STRING_SIZE - 1; i++) {
        key[i] = keySet[keySeed[i] % (sizeof(keySet) - 1)];
    }
    key[STRING_SIZE - 1] = NULL;
}