
#ifndef KORE_RESULT_OK
#include <kore/kore.h>
#include <kore/http.h>
#endif

#ifndef KORE_RESULT_OK
#include <kore.h>
#include <http.h>
#endif

#include "assets.h"
#include "../models/models.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <openssl/sha.h>
#include <time.h>

#define class struct
#define null NULL

int page(struct http_request *);
int login(struct http_request *);
int createUser(struct http_request *);

int page(struct http_request *req)
{
	struct kore_buf *buffer;
	struct http_cookie *cookie;
	char *value;

	/* set formatted cookie */

	http_populate_cookies(req);

	buffer = getCookieValue(req, "Lando");
	if (buffer != null)	{
		kore_log(2, buffer->data);
	}

	setCookie(req, "Lando", "TopKEK", "/");
	http_populate_get(req);

	buffer = kore_buf_alloc(asset_len_MasterPage_html);

	kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);
	kore_buf_replace_string(buffer, "$body$", asset_Index_html, asset_len_Index_html);

	http_response(req, 200, buffer->data, buffer->offset);

	return (KORE_RESULT_OK);
}

int login(struct http_request *req)
{
	struct kore_buf *buffer;
	char *firstName;
	char *lastName;

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	char *server = "127.0.0.1";
	char *user = "root";
	char *password = "root"; /* set me first */
	char *database = "sys";

	kore_log(2, mysql_get_client_info());

	/* Connect to database */
	if (mysql_real_connect(conn, server,
						   user, password, database, 3306, NULL, 0))
	{
		kore_log(2, mysql_error(conn));
	}
	mysql_select_db(conn, "sys");

	mysql_query(conn, "select * from new_table;");
	//mysql_query(conn, "show tables");

	MYSQL_RES *result = mysql_store_result(conn);
	if (result == NULL)
	{
		kore_log(2, "nothing found");
	}
	else
	{
		MYSQL_ROW row;

		kore_log(2, "result is not null");
		unsigned long *lengths = mysql_fetch_lengths(result);
		kore_log(2, "MySQL Tables in mysql database:");

		while ((row = mysql_fetch_row(result)))
		{
			kore_log(2, row[1]);
		}

		//while ((row = mysql_fetch_row(res)) != NULL) {
		//	kore_log(2, row[0]);
		//}
	}

	kore_log(2, mysql_stat(conn));
	if (req->method == HTTP_METHOD_POST)
	{

		http_populate_post(req);
		http_argument_get_string(req, "firstname", &firstName);
		http_argument_get_string(req, "lastname", &lastName);

		buffer = kore_buf_alloc(asset_len_MasterPage_html);
		kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

		char *boi = "<iframe width=\"560\" height=\"315\" src=\"https://www.youtube.com/embed/TCCJOTY7uRI?autoplay=1\" frameborder=\"0\" allowfullscreen></iframe>";

		//kore_buf_replace_string(buffer, "$body$", asset_Login_html, asset_len_Login_html);
		kore_buf_replace_string(buffer, "$body$", boi, strlen(boi));

		//char * script = "var page = new Vue({ \
		//					el: '#error', \
		//					data : { \
		//					visible: true \
		//					} \
		//					}) \
		//					";
		//kore_buf_replace_string(buffer, "$script$", script, strlen(script));

		http_response(req, 200, buffer->data, buffer->offset);
		return (KORE_RESULT_OK);
	}
	else
	{
		buffer = kore_buf_alloc(asset_len_MasterPage_html);
		kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

		kore_buf_replace_string(buffer, "$body$", asset_Login_html, asset_len_Login_html);

		char *script = "var page = new Vue({ \
							el: '#error', \
							data : { \
							visible: false \
							} \
							}) \
							";
		kore_buf_replace_string(buffer, "$script$", script, strlen(script));

		http_response(req, 200, buffer->data, buffer->offset);
		return (KORE_RESULT_OK);
	}
}

int createUser(struct http_request *req)
{

	http_response(req, 200, NULL, NULL);
	return (KORE_RESULT_OK);
}

int flightOverView(struct http_request *req)
{

	struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
	kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

	kore_buf_replace_string(buffer, "$links$", asset_Links_html, asset_len_Links_html);
	kore_buf_replace_string(buffer, "$body$", asset_FlightOverview_html, asset_len_FlightOverview_html);
	http_response(req, 200, NULL, NULL);
	return (KORE_RESULT_OK);
}