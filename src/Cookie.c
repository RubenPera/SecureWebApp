#include "Header.h"

void setCookie(struct http_request *req, char *name, char *value, char *path) {
    struct kore_buf *buffer;
    char *initialValue = "$name$=$value$; Path=$path$; HttpOnly; Secure; SameSite=strict;";
    buffer = kore_buf_alloc(strlen(initialValue));
    kore_buf_append(buffer, initialValue, strlen(initialValue));

    kore_buf_replace_string(buffer, "$name$", name, strlen(name));
    kore_buf_replace_string(buffer, "$value$", value, strlen(value));
    kore_buf_replace_string(buffer, "$path$", path, strlen(path));

    http_response_header(req, "set-cookie", buffer->data);
}

//TODO: should be decrypted
struct kore_buf *getCookieValue(struct http_request *req, char *name) {
    char *value;
    struct kore_buf *buffer;
    if (http_request_cookie(req, name, &value)) {
        buffer = kore_buf_alloc(strlen(value));
        kore_buf_append(buffer, value, strlen(value));
        return buffer;
    } else {
        return NULL;
    }
}

// Creates Session Cookie
void createSessionCookie(struct http_request *req, int user_id) {
    char key[STRING_SIZE];

    generateSessionKey(key);

    createSessionRow(user_id, key);
    setCookie(req, "session", key, "/");
//    kore_log(2, "sizeof key = %d key = %s",sizeof(key), key);
    getUserIdWithSession(key);
}

void generateSessionKey(char key[STRING_SIZE]) {
    char keySeed[STRING_SIZE];

    const char keySet[] = {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()-_=+,<.>/?`~"};

    generate_random(keySeed, STRING_SIZE);
    for (int i = 0; i < STRING_SIZE - 1; i++) {
        key[i] = keySet[keySeed[i] % sizeof(keySet)];

    }
    key[STRING_SIZE - 1] = NULL;

}