
#include "Header.h"


// Returns userId if a user is logged in
// Returns false if a user is not logged in
int getLoggedInUser(struct http_request *req) {
    http_populate_cookies(req);

    struct kore_buf *buffer;
    buffer = getSessionCookieValue(req);
    if (buffer != NULL && buffer->data != NULL) {
        return getUserIdWithSession(buffer->data);
    }
    return false;
}

int page(struct http_request *req) {
    int userId = getLoggedInUser(req);
    if (userId) {
        struct kore_buf *cookieValue;

        cookieValue = getSessionCookieValue(req);

        update_session(cookieValue->data);

        struct kore_buf *buffer;
        http_populate_cookies(req);
        buffer = kore_buf_alloc(null);

        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);
        kore_buf_replace_string(buffer, "$body$", asset_Index_html, asset_len_Index_html);
        http_response(req, 200, buffer->data, buffer->offset);
        return (KORE_RESULT_OK);
    } else {
        return login(req);
    }
}

// Returns the Login page
int login(struct http_request *req) {
    struct kore_buf *buffer;
    char *login_email_param = NULL;
    char *login_password_param = NULL;

    if (getLoggedInUser(req)) {
        return page(req);
    }

    if (req->method == HTTP_METHOD_POST) {
        http_populate_post(req);
        if (KORE_RESULT_OK == http_argument_get_string(req, "email", &login_email_param) &&
            KORE_RESULT_OK == http_argument_get_string(req, "password", &login_password_param)) {

            DatabaseResult dbResult;
            dbResult = getIdSaltHashWithEmail(login_email_param);

            bool temp = login_validate_password(login_password_param, get_DatabaseResult(dbResult, 0, 2),
                                                get_DatabaseResult(dbResult, 0, 1));
            if (!temp) {
                return showLoginPage(req);
            }

            http_populate_cookies(req);
            createSessionCookie(req, (int) get_DatabaseResult(dbResult, 0, 0));

            buffer = kore_buf_alloc(asset_len_MasterPage_html);
            kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

            char *boi = "<iframe width=\"560\" height=\"315\" src=\"https://www.youtube.com/embed/TCCJOTY7uRI?autoplay=1\" frameborder=\"0\" allowfullscreen></iframe>";

            kore_buf_replace_string(buffer, "$body$", boi, strlen(boi));
            http_response(req, 200, buffer->data, buffer->offset);
            return (KORE_RESULT_OK);
        }
    }
    return showLoginPage(req);
}

int createUser(struct http_request *req) {

    http_response(req, 200, NULL, NULL);
    return (KORE_RESULT_OK);
}

int flightOverView(struct http_request *req) {
    int userId = getLoggedInUser(req);
    if (userId) {
        struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

        kore_buf_replace_string(buffer, "$links$", asset_Links_html, asset_len_Links_html);
        kore_buf_replace_string(buffer, "$body$", asset_FlightOverview_html, asset_len_FlightOverview_html);
        http_response(req, 200, buffer->data, buffer->offset);
        return (KORE_RESULT_OK);
    } else {
        return login(req);
    }
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
    int userId = getLoggedInUser(req);
//    kore_log(2, " test, %s", (char*)userId);
    if (userId && req->method == HTTP_METHOD_POST) {
        http_populate_post(req);


        /* Grab it as a string, we shouldn't free the result in sid. */
        if (http_argument_get_string(req, "id", &sid)) {
            createBooking(userId, sid);
        }

        /* Grab it as an actual u_int16_t. */
        if (http_argument_get_uint16(req, "id", &id)) {

        }
        http_response(req, 200, NULL, NULL);

        return (KORE_RESULT_OK);
    } else {
        return (KORE_RESULT_ERROR);
    }
}

validate_password_regex() {
    // ^.*(?=.{12,})(?=.*[a-zA-Z])(?=.*\d)(?=.*[!#$%&? "])(?=.*[A-Z]).*$
}

// Returns the Login page
// Logs out the currenty logged in user by removing the session cookie
int logout(struct http_request *req) {
    DatabaseResult dbResult;

    if (getLoggedInUser(req)) {
        removeSessionCookie(req);
    }
    return showLoginPage(req);
}

int showLoginPage(struct http_request *req) {
    struct kore_buf *buffer;
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