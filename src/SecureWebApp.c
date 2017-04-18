
#ifndef KORE_RESULT_OK 
#include <kore/kore.h>
#include <kore/http.h>
#endif

#ifndef KORE_RESULT_OK 
#include <kore.h>
#include <http.h>
#endif

#include "assets.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <openssl/sha.h>


const char * htmlEnd = "</script></body></html>";

int	page(struct http_request *);
int login(struct http_request *);
void readHtmlFileIntoBuffer(struct kore_buf * buffer, char * htmlPage, char * vueValue);

int page(struct http_request *req)
{
	struct kore_buf	*buffer;
	kore_log(2, "HELP");
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

	MYSQL *conn;
   	MYSQL_RES *res;
   	MYSQL_ROW row;

   char *server = "172.0.0.1";
   char *user = "root";
   char *password = "root"; /* set me first */
   char *database = "mysql";

   conn = mysql_init(NULL);

   /* Connect to database */
   if (! mysql_real_connect(conn, server,
         user, password, database, 3306, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   }

	buffer = kore_buf_alloc(128);
	kore_log(2, "test");

	if (req->method == HTTP_METHOD_POST) {
		http_populate_get(req);

		http_populate_post(req);
		http_argument_get_string(req, "firstname", &firstName);
		http_argument_get_string(req, "lastname", &lastName);
		kore_log(2, firstName);
		kore_log(2, lastName);

		buffer = kore_buf_alloc(asset_len_Index_html);
		kore_buf_append(buffer, asset_Index_html, asset_len_Index_html);

		kore_buf_replace_string(buffer, "$firstname$", firstName, strlen(firstName));

		http_response(req, 200, buffer->data, buffer->offset);
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

