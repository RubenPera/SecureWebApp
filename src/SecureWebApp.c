
#ifndef KORE_RESULT_OK 
#include <kore/kore.h>
#include <kore/http.h>
#endif

#ifndef KORE_RESULT_OK 
#include <kore.h>
#include <http.h>
#endif

#include "assets.h"
//#include <mysql/mysql.h>
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

	buffer = kore_buf_alloc(asset_len_Index_html);
	kore_buf_append(buffer, asset_Index_html, asset_len_Index_html);

	http_response(req, 200, buffer->data, buffer->offset);
	return (KORE_RESULT_OK);
}

int login(struct http_request *req)
{
	struct kore_buf	*buffer;
	char			*firstName;
	char * lastName;

	//MYSQL *conn;
 //  	MYSQL_RES *res;
 //  	MYSQL_ROW row;

 //  char *server = "172.0.0.1";
 //  char *user = "root";
 //  char *password = "root"; /* set me first */
 //  char *database = "mysql";

 //  conn = mysql_init(NULL);

 //  /* Connect to database */
 //  if (! mysql_real_connect(conn, server,
 //        user, password, database, 3306, NULL, 0)) {
 //     fprintf(stderr, "%s\n", mysql_error(conn));
 //     exit(1);
 //  }
	if (req->method == HTTP_METHOD_POST) {
		http_populate_get(req);

		http_populate_post(req);
		http_argument_get_string(req, "firstname", &firstName);
		http_argument_get_string(req, "lastname", &lastName);

		buffer = kore_buf_alloc(asset_len_Index_html);
		kore_buf_append(buffer, asset_Index_html, asset_len_Index_html);

		kore_buf_replace_string(buffer, "$firstname$", firstName, strlen(firstName));

		http_response(req, 200, buffer->data, buffer->offset);
		return (KORE_RESULT_OK);
	}
	else {
		buffer = kore_buf_alloc(asset_len_Login_html);
		kore_buf_append(buffer, asset_Login_html, asset_len_Login_html);

		http_response(req, 200, buffer->data, buffer->offset);
		return (KORE_RESULT_OK);
	}
}
