
#include "Header.h"

int page(struct http_request *req)
{
	struct kore_buf *buffer;
	struct http_cookie *cookie;
	char *value;
	DatabaseResult dbResult = getUsers();
	/* set formatted cookie */

	http_populate_cookies(req);

    buffer = getCookieValue(req, "Lando");
    if (buffer != null)
    {
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

    // kore_log(2, mysql_stat(conn));
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
    http_response(req, 200, buffer->data, buffer->offset);
    return (KORE_RESULT_OK);
}

int getFlights(struct http_request *req)
{
    SmartString *ss = smart_string_new();
    smart_string_append(ss, "{\"Flights\":[");
    smart_string_append(ss, "{\"id\":\"1\",\"from\":\"Utrecht\", \"to\": \"Amsterdam\",\"date\":\"21-03-2017\",\"airmiles\":\"5231\"},");
    smart_string_append(ss, "{\"id\":\"1\",\"from\":\"Misteryland\", \"to\": \"Amsterdam\",\"date\":\"21-03-2017\",\"airmiles\":\"5231\"},");
    smart_string_append(ss, "{\"id\":\"1\",\"from\":\"Misteryland\", \"to\": \"Utrecht\",\"date\":\"21-03-2017\",\"airmiles\":\"7201\"}");
    smart_string_append(ss, "]}");

    http_response(req, 200, ss->buffer, (unsigned)strlen(ss->buffer));

    return (KORE_RESULT_OK);
}



