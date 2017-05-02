
#ifndef KORE_RESULT_OK
#include <kore/kore.h>
#include <kore/http.h>
#endif

#ifndef KORE_RESULT_OK
#include <kore.h>
#include <http.h>
#endif

void setCookie(struct http_request * req, char * name, char * value, char * path);
struct kore_buf	* getCookieValue(struct http_request * req, char * name);

// TODO: should be encrypted

void setCookie(struct http_request * req, char * name, char * value, char * path){
    struct kore_buf *buffer;
    char * initialValue = "$name$=$value$; Path=$path$; HttpOnly; Secure;";
    buffer = kore_buf_alloc(strlen(initialValue));
    kore_buf_append(buffer, initialValue, strlen(initialValue));

    kore_buf_replace_string(buffer, "$name$", name, strlen(name));
    kore_buf_replace_string(buffer, "$value$", value, strlen(value));
    kore_buf_replace_string(buffer, "$path$", path, strlen(path));

    http_response_header(req, "set-cookie", buffer->data); 
}
//TODO: should be decrypted
struct kore_buf	* getCookieValue(struct http_request * req, char * name){
    char * value;
    struct kore_buf * buffer;
    if (http_request_cookie(req, name, &value))
	{
		buffer = kore_buf_alloc(strlen(value));
        kore_buf_append(buffer, value, strlen(value));
        return buffer;
	} else{
        return NULL;
    }
}