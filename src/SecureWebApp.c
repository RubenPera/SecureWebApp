
#ifndef KORE_RESULT_OK 
#include <kore/kore.h>
#include <kore/http.h>
#endif

#ifndef KORE_RESULT_OK 
#include <kore.h>
#include <http.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <openssl/sha.h>


const char * htmlEnd = "    </script></body></html>";

int	page(struct http_request *);
int login(struct http_request *);
void readHtmlFileIntoBuffer(struct kore_buf * buffer, char * htmlPage, char * vueValue);

int page(struct http_request *req)
{
	struct kore_buf	*buffer;

	http_populate_get(req);
	buffer = kore_buf_alloc(128);

	char * htmlPage = "Index.html";
	readHtmlFileIntoBuffer(buffer, htmlPage, NULL);

	http_response(req, 200, buffer->data, buffer->offset);
	return (KORE_RESULT_OK);
}

int login(struct http_request *req)
{
	struct kore_buf	*buffer;
	char			*firstName;
	char * lastName;

	buffer = kore_buf_alloc(128);
	kore_log(2, "test");

	if (req->method == HTTP_METHOD_POST) {
		http_populate_get(req);

		http_populate_post(req);
		http_argument_get_string(req, "firstname", &firstName);
		http_argument_get_string(req, "lastname", &lastName);
		kore_log(2, firstName);
		kore_log(2, lastName);

		char * temp = "var app = new Vue({ \
		el: '#app', \
			data : { \
		messagetest: 'Hello World', \
			test : 'hello GLENNOS' \
		} \
		})";
		http_response(req, 200, firstName, strlen(firstName));
		return (KORE_RESULT_OK);
	}
	else {

		char * htmlPage = "Login.html";
		readHtmlFileIntoBuffer(buffer, htmlPage, NULL);
		http_response(req, 200, buffer->data, buffer->offset);
		return (KORE_RESULT_OK);
	}
}



void readHtmlFileIntoBuffer(struct kore_buf * buffer, char * htmlPage, char * vueValue) {
	long length;
	char * buf;

	char view[80] = "view/";
	strcat(view, htmlPage);
	FILE *file = fopen(view, "rb");

	fseek(file, 0, SEEK_END);
	length = ftell(file);

	fseek(file, 0, SEEK_SET);
	buf = malloc(length);
	if (buf)
	{
		fread(buf, 1, length, file);
	}
	fclose(file);
	kore_buf_append(buffer, buf, length);
	if (vueValue != NULL) {
		kore_buf_append(buf, vueValue, strlen(vueValue));
	}
	kore_buf_append(buffer, htmlEnd, strlen(htmlEnd));
}

