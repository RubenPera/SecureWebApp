
#include "Header.h"

bool isLoggedIn(struct http_request *req) {
    http_populate_cookies(req);
    struct kore_buf *buffer;
    buffer = getCookieValue(req, "session");
    if (buffer != NULL && buffer->data != NULL) {
        DatabaseResult dbResult;
        kore_log(2, "buffer %s", buffer->data);
        dbResult = getUserIdWithSession(buffer->data);
        if (get_DatabaseResult(dbResult, 0, 0) != NULL) {
            return true;
        }
    }
    return false;
}

int page(struct http_request *req) {
    if (isLoggedIn(req)) {
        struct kore_buf *buffer;
        struct http_cookie *cookie;
        char *value;
        DatabaseResult dbResult;
        http_populate_cookies(req);
        createSessionCookie(req, 123);
        buffer = kore_buf_alloc(null);
        dbResult = getSaltHashWithEmail("r.@gmail.com");
        kore_log(2, "%s", get_DatabaseResult(dbResult, 0, 0));
        kore_log(2, "%s", get_DatabaseResult(dbResult, 0, 1));

        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);
        kore_buf_replace_string(buffer, "$body$", asset_Index_html, asset_len_Index_html);
        http_response(req, 200, buffer->data, buffer->offset);
        return (KORE_RESULT_OK);
    } else {
        return login(req);
    }
}

int login(struct http_request *req) {
    struct kore_buf *buffer;
    char *login_email_param = NULL;
    char *login_password_param = NULL;

    // kore_log(2, mysql_stat(conn));
    if (req->method == HTTP_METHOD_POST) {
        http_populate_post(req);
        kore_log(2, "result = %d", http_argument_get_string(req, "email", &login_email_param));
        if (KORE_RESULT_OK == http_argument_get_string(req, "email", &login_email_param) &&
            KORE_RESULT_OK == http_argument_get_string(req, "password", &login_password_param)) {

            kore_log(2, "%s", login_email_param);
            DatabaseResult dbResult;
            dbResult = getSaltHashWithEmail(login_email_param);

            kore_log(2, "%s", get_DatabaseResult(dbResult, 0, 0));
            kore_log(2, "%s", get_DatabaseResult(dbResult, 0, 1));
            http_populate_cookies(req);
//
//            buffer = getCookieValue(req, "session");
//            int temp = (int)(buffer->data - '0');
//            kore_log(2, "cookie value = %d", temp);

            createSessionCookie(req, 1);

        }
        buffer = kore_buf_alloc(asset_len_MasterPage_html);
        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

        char *boi = "<iframe width=\"560\" height=\"315\" src=\"https://www.youtube.com/embed/TCCJOTY7uRI?autoplay=1\" frameborder=\"0\" allowfullscreen></iframe>";

        //kore_buf_replace_string(buffer, "$body$", asset_Login_html, asset_len_Login_html);
        kore_buf_replace_string(buffer, "$body$", boi, strlen(boi));
        http_response(req, 200, buffer->data, buffer->offset);
        return (KORE_RESULT_OK);
    } else {
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

int createUser(struct http_request *req) {

    http_response(req, 200, NULL, NULL);
    return (KORE_RESULT_OK);
}

int flightOverView(struct http_request *req) {

    struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
    kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

    kore_buf_replace_string(buffer, "$links$", asset_Links_html, asset_len_Links_html);
    kore_buf_replace_string(buffer, "$body$", asset_FlightOverview_html, asset_len_FlightOverview_html);
    http_response(req, 200, buffer->data, buffer->offset);
    return (KORE_RESULT_OK);
}

int getFlights(struct http_request *req) {
    char *query = "call get_all_flights()";
    char *groupname = "Flights";
    SmartString *str = smart_string_new();
    sqlToJson(str, query, groupname);

    /*Send data to page - response */
    http_response_header(req, "content-type", "application/json");
    http_response(req, 200, str->buffer, (unsigned) strlen(str->buffer));

    /*Clean up smartstring - free up memory*/
    smart_string_destroy(str);

    return (KORE_RESULT_OK);
}

int bookFlight(struct http_request *req) {
    u_int16_t id;
    char *sid;
    struct kore_buf *buf;

    http_populate_post(req);

    /* Grab it as a string, we shouldn't free the result in sid. */
    if (http_argument_get_string(req, "id", &sid)) {
        createBooking("1", sid);
    }

    /* Grab it as an actual u_int16_t. */
    if (http_argument_get_uint16(req, "id", &id)) {
    }
    http_response(req, 200, NULL, NULL);

    return (KORE_RESULT_OK);
}

validate_password_regex() {
    // ^.*(?=.{12,})(?=.*[a-zA-Z])(?=.*\d)(?=.*[!#$%&? "])(?=.*[A-Z]).*$
}