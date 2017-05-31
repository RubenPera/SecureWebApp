
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

            bool isAuthenticated = login_validate_password(login_password_param, get_DatabaseResult(dbResult, 0, 2),
                                                get_DatabaseResult(dbResult, 0, 1));
            if (!isAuthenticated) {
                http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
                return (KORE_RESULT_OK);
            }

            http_populate_cookies(req);
            createSessionCookie(req, (int) get_DatabaseResult(dbResult, 0, 0));

            http_response(req, 200, NULL, NULL);
            return (KORE_RESULT_OK);
        } else {
            http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
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
    char *groupName = "Flights";
    SmartString *str = smart_string_new();
    sqlToJson(str, query, groupName);

    /*Send data to page - response */
    http_response_header(req, "content-type", "application/json");
    http_response(req, 200, str->buffer, (unsigned) strlen(str->buffer));

    /*Clean up smartstring - free up memory*/
    smart_string_destroy(str);

    return (KORE_RESULT_OK);
}

int getLinks(struct http_request *req) {
    int userId = getLoggedInUser(req);
    kore_log(2, "userId = %d", userId);
    if (userId) {

        DatabaseResult dbResult = getUserWithId(userId);
        int role = (int) get_DatabaseResult(dbResult, 0, 5);
        kore_log(2, "Role = %d", role);

        SmartString *str = smart_string_new();

        /*Creating a json object*/
        json_object *container = json_object_new_object();
        /*Creating a json array*/

        char *email = get_DatabaseResult(dbResult, 0, db_user_email);
        int miles = (int) get_DatabaseResult(dbResult, 0, db_user_inholland_miles);

        char strMiles[sizeof(miles)];
        sprintf(strMiles, "%d", miles);

        kore_log(2, "test %s", strMiles);
        SmartString *userInfo = smart_string_new();
        smart_string_append(userInfo, email);
        smart_string_append(userInfo, " | ");
        smart_string_append(userInfo, strMiles);

        kore_log(2, "test %s", userInfo->buffer);

        if (role == 1) {
            // admin
            char *texts[] = {"Flights", "Admin panel" "Logout",};
            char *links[] = {"flightOverView", "", "logout"};
            fillLinks(container, (sizeof(texts) / sizeof(char *)), texts, (sizeof(links) / sizeof(char *)), links);
        } else if (role == 0) {
            // normal user
            char *texts[] = {"Flights", "Logout", (char *) userInfo->buffer};
            char *links[] = {"flightOverView", "logout", "userInfo"};
            fillLinks(container, (sizeof(texts) / sizeof(char *)), texts, (sizeof(links) / sizeof(char *)), links);
        }
        smart_string_append(str, json_object_to_json_string(container));

        setCookie(req, "test", "test");

        http_response_header(req, "content-type", "application/json");

        http_response(req, 200, str->buffer, (
                unsigned) strlen(str->buffer));

        smart_string_destroy(str);
        return (KORE_RESULT_OK);
    } else {
        http_response(req, 200, NULL, NULL);
        return (KORE_RESULT_OK);
    }
}

void fillLinks(json_object *container, int sizeTexts, char *texts[sizeTexts], int sizeLinks, char *links[sizeLinks]) {
    json_object *jsonLinks = json_object_new_array();

    for (int i = 0; i < sizeTexts; i++) {
        json_object *text = json_object_new_string(texts[i]);
        json_object *link = json_object_new_string(links[i]);

        json_object *item = json_object_new_object();
        json_object_object_add(item, "link", link);
        json_object_object_add(item, "text", text);

        json_object_array_add(jsonLinks, item);
    }

    json_object_object_add(container, "Links", jsonLinks);

}

int getLoggedInUserName(struct http_request *req) {
    int userId = getLoggedInUser(req);
    if (userId) {

    } else {
        http_response(req, 200, NULL, NULL);
        return (KORE_RESULT_OK);
    }

}

int bookFlight(struct http_request *req) {
    u_int16_t id;

    int userId = getLoggedInUser(req);
    if (userId) {
        http_populate_post(req);

        /* Grab it as an actual u_int16_t. */
        if (http_argument_get_uint16(req, "id", &id)) {
            createBooking(userId, id);
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

    http_response(req, 200, buffer->data, buffer->offset);
    return (KORE_RESULT_OK);
}


int userInfo(struct http_request *req) {

    struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
    kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

    kore_buf_replace_string(buffer, "$links$", asset_Links_html, asset_len_Links_html);
    kore_buf_replace_string(buffer, "$body$", asset_UserInfo_html, asset_len_UserInfo_html);
    http_response(req, 200, buffer->data, buffer->offset);
    return (KORE_RESULT_OK);
}

int getUserInfo(struct http_request *req) {
    u_int16_t id;
    char *sid;
    struct kore_buf *buf;

    http_populate_get(req);

    /* Grab it as a string, we shouldn't free the result in sid. */
    if (http_argument_get_string(req, "id", &sid)) {
        SmartString *query = smart_string_new();
        smart_string_append(query, "call get_userinfo(");
        smart_string_append(query, sid);
        smart_string_append(query, ");");

        kore_log(2, query->buffer);

        char *groupname = "Users";
        SmartString *str = smart_string_new();
        sqlToJson(str, query->buffer, groupname);

        /*Send data to page - response */
        http_response_header(req, "content-type", "application/json");
        http_response(req, 200, str->buffer, (unsigned) strlen(str->buffer));

        kore_log(2, str->buffer);


        /*Clean up smartstring - free up memory*/
        smart_string_destroy(str);
        smart_string_destroy(query);

    }

    return (KORE_RESULT_OK);
}

int getFlightsBooked(struct http_request *req) {
    u_int16_t id;
    char *sid;
    struct kore_buf *buf;

    http_populate_get(req);

    /* Grab it as a string, we shouldn't free the result in sid. */
    if (http_argument_get_string(req, "id", &sid)) {
        SmartString *query = smart_string_new();
        smart_string_append(query, "call get_bookedflights(");
        smart_string_append(query, sid);
        smart_string_append(query, ");");

        kore_log(2, query->buffer);

        char *groupname = "Flights";
        SmartString *str = smart_string_new();
        sqlToJson(str, query->buffer, groupname);

        /*Send data to page - response */
        http_response_header(req, "content-type", "application/json");
        http_response(req, 200, str->buffer, (unsigned) strlen(str->buffer));

        kore_log(2, str->buffer);


        /*Clean up smartstring - free up memory*/
        smart_string_destroy(str);
        smart_string_destroy(query);

    }

    return (KORE_RESULT_OK);
}

int serve_css(struct http_request *req) {

    http_response_header(req, "content-type", "text/css");
    http_response(req, 200, asset_style_css, asset_len_style_css);

    return (KORE_RESULT_OK);
}

int serve_js(struct http_request * req){

    http_response_header(req, "content-type", "text/javascript");
    http_response(req, 200, asset_code_js, asset_len_code_js);

    return (KORE_RESULT_OK);
}