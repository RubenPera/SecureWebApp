#include "Header.h"

void setCookie(struct http_request *req, char *name, char *value, char *path)
{
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
struct kore_buf *getCookieValue(struct http_request *req, char *name)
{
    char *value;
    struct kore_buf *buffer;
    if (http_request_cookie(req, name, &value))
    {
        buffer = kore_buf_alloc(strlen(value));
        kore_buf_append(buffer, value, strlen(value));
        return buffer;
    }
    else
    {
        return NULL;
    }
}

// Creates Session Cookie
void createSessionCookie(struct http_request *req, int user_id)
{
    char *key;
    int size_of_key = 16;
    key = (char *)malloc(size_of_key * sizeof(char));

    if (!RAND_bytes(key, sizeof key))
    {
        kore_log(2, "openssl error");
    }
    setCookie(req, "session", key, "/");

    
}