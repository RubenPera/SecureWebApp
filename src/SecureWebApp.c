
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
    int userId = getLoggedInUser(req);
    if (userId) {

        DatabaseResult newDbResult = getAllFlights();
        SmartString *str = smart_string_new();
        /*Creating a json object*/
        json_object *container = json_object_new_object();
        /*Creating a json array*/
        json_object *flights = json_object_new_array();
        for (unsigned int row = 0; row < newDbResult.rows; row++) {

            json_object *dateValue = json_object_new_string(get_DatabaseResult(newDbResult, row, db_flight_date));

            json_object *priceValue = json_object_new_int((int) get_DatabaseResult(newDbResult, row, db_flight_price));
            json_object *capacityValue = json_object_new_int(
                    (int) get_DatabaseResult(newDbResult, row, db_flight_capacity));

            json_object *externalIdValue = json_object_new_int(
                    (int) get_DatabaseResult(newDbResult, row, db_flight_external_id));

            json_object *sourceValue = json_object_new_string(
                    get_DatabaseResult(newDbResult, row, db_flight_flight_source));
            json_object *destinationValue = json_object_new_string(
                    get_DatabaseResult(newDbResult, row, db_flight_flight_destination));


            json_object *flight = json_object_new_object();

            json_object_object_add(flight, "date", dateValue);
            json_object_object_add(flight, "price", priceValue);
            json_object_object_add(flight, "capacity", capacityValue);

            json_object_object_add(flight, "flight_destination", destinationValue);
            json_object_object_add(flight, "flight_source", sourceValue);
            json_object_object_add(flight, "external_id", externalIdValue);

            json_object_array_add(flights, flight);
        }

        json_object_object_add(container, "Flights", flights);

        smart_string_append(str, json_object_to_json_string(container));

        http_response_header(req, "content-type", "application/json");

        http_response(req, 200, str->buffer, (
                unsigned) strlen(str->buffer));

        smart_string_destroy(str);
        return (KORE_RESULT_OK);
    }
    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}

int getLinks(struct http_request *req) {
    int userId = getLoggedInUser(req);
    kore_log(2, "userId = %d", userId);
    if (userId) {
        DatabaseResult dbResult = getUserWithId(userId);
        int role = (int) get_DatabaseResult(dbResult, 0, db_user_role);

        SmartString *str = smart_string_new();

        /*Creating a json object*/
        json_object *container = json_object_new_object();
        /*Creating a json array*/

        char *email = get_DatabaseResult(dbResult, 0, db_user_email);
        int miles = (int) get_DatabaseResult(dbResult, 0, db_user_inholland_miles);

        char strMiles[sizeof(miles)];
        sprintf(strMiles, "%d", miles);


        SmartString *userInfo = smart_string_new();
        smart_string_append(userInfo, email);
        smart_string_append(userInfo, " | ");
        smart_string_append(userInfo, strMiles);

        if (role == adminRole) {
            // admin
            char *texts[] = {"Flights", "Users", "Logout"};
            char *links[] = {"flightOverView", "adminUsers", "logout"};
            fillLinks(container, (sizeof(texts) / sizeof(char *)), texts, (sizeof(links) / sizeof(char *)), links);
        } else {
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


int userInfo(struct http_request *req)
{

    int userId = getLoggedInUser(req);

    if (userId && req->method == HTTP_METHOD_GET)
    {
        struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

        kore_buf_replace_string(buffer, "$links$", asset_Links_html, asset_len_Links_html);
        kore_buf_replace_string(buffer, "$body$", asset_UserInfo_html, asset_len_UserInfo_html);
        http_response(req, 200, buffer->data, buffer->offset);

        return (KORE_RESULT_OK);
    }
    else
    {
        kore_log(2,"Unauthorized User Access");
        return login(req);
    }
}

int getUserInfo(struct http_request *req)
{
    http_populate_get(req);

    int userId = getLoggedInUser(req);

    if (userId && req->method == HTTP_METHOD_GET)
    {
        SmartString *query = smart_string_new();
        smart_string_append(query, "call get_userinfo(");
        smart_string_append_sprintf(query, "%d", userId);
        smart_string_append(query, ");");

        kore_log(2, query->buffer);

        char *groupname = "Users";
        SmartString *str = smart_string_new();
        sqlToJson(str, query->buffer, groupname);

        /*Send data to page - response */
        http_response_header(req, "content-type", "application/json");
        http_response(req, 200, str->buffer, (unsigned)strlen(str->buffer));

        kore_log(2, str->buffer);

        /*Clean up smartstring - free up memory*/
        smart_string_destroy(str);
        smart_string_destroy(query);
    }
    else
    {
        http_response(req, 401,"Unauthorized", (unsigned)strlen("Unauthorized"));
        kore_log(2,"Unauthorized User Access");
        return login(req);

    }

    return (KORE_RESULT_OK);
}

int getFlightsBooked(struct http_request *req)
{
    http_populate_get(req);

    int userId = getLoggedInUser(req);

    if (userId && req->method == HTTP_METHOD_GET)
    {
        SmartString *query = smart_string_new();
        smart_string_append(query, "call get_bookedflights(");
        smart_string_append_sprintf(query, "%d", userId);
        smart_string_append(query, ");");

        kore_log(2, query->buffer);

        char *groupname = "Flights";
        SmartString *str = smart_string_new();
        sqlToJson(str, query->buffer, groupname);

        /*Send data to page - response */
        http_response_header(req, "content-type", "application/json");
        http_response(req, 200, str->buffer, (unsigned)strlen(str->buffer));

        kore_log(2, str->buffer);

        /*Clean up smartstring - free up memory*/
        smart_string_destroy(str);
        smart_string_destroy(query);
    }
    else
    {
        http_response(req, 401,"Unauthorized", (unsigned)strlen("Unauthorized"));
        kore_log(2,"Unauthorized User Access");
        return login(req);
    }

    return (KORE_RESULT_OK);
}

int adminGetUsers(struct http_request *req) {
    int userId = getLoggedInUser(req);
    if (userId) {
        DatabaseResult dbResult = getUserWithId(userId);
        int role = (int) get_DatabaseResult(dbResult, 0, db_user_role);
        if (role == adminRole) {
            DatabaseResult newDbResult = getAllUsers();
            SmartString *str = smart_string_new();
            /*Creating a json object*/
            json_object *container = json_object_new_object();
            /*Creating a json array*/
            json_object *users = json_object_new_array();
            for (unsigned int row = 0; row < newDbResult.rows; row++) {
                json_object *emailValue = json_object_new_string(get_DatabaseResult(newDbResult, row, db_user_email));

                json_object *miles = json_object_new_int(
                        (int) get_DatabaseResult(newDbResult, row, db_user_inholland_miles));
                json_object *userRole = json_object_new_int((int) get_DatabaseResult(newDbResult, row, db_user_role));
                json_object *newMiles = json_object_new_int(0);

                json_object *user = json_object_new_object();

                json_object_object_add(user, "email", emailValue);
                json_object_object_add(user, "miles", miles);
                json_object_object_add(user, "role", userRole);
                json_object_object_add(user, "newMiles", newMiles);

                json_object_array_add(users, user);
            }

            json_object_object_add(container, "Users", users);

            smart_string_append(str, json_object_to_json_string(container));

            http_response_header(req, "content-type", "application/json");

            http_response(req, 200, str->buffer, (
                    unsigned) strlen(str->buffer));

            smart_string_destroy(str);
            return (KORE_RESULT_OK);
        }
    }
    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}

int adminUsers(struct http_request *req) {
    int userId = getLoggedInUser(req);
    if (userId) {
        DatabaseResult dbResult = getUserWithId(userId);
        int role = (int) get_DatabaseResult(dbResult, 0, db_user_role);
        if (role == adminRole) {
            struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
            kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

            kore_buf_replace_string(buffer, "$body$", asset_Users_html, asset_len_Users_html);
            http_response(req, 200, buffer->data, buffer->offset);
            return (KORE_RESULT_OK);
        }
    }
    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}

int adminSetNewAirMilesValue(struct http_request *req) {
    char *email_param = NULL;
    char *airmiles_param = NULL;
    int adminId = getLoggedInUser(req);

    if (adminId && req->method == HTTP_METHOD_POST) {
        DatabaseResult dbResult = getUserWithId(adminId);
        int role = (int) get_DatabaseResult(dbResult, 0, db_user_role);
        if (role == adminRole) {
            http_populate_post(req);
            if (KORE_RESULT_OK == http_argument_get_string(req, "email", &email_param) &&
                KORE_RESULT_OK == http_argument_get_string(req, "airmiles", &airmiles_param)) {

                kore_log(2, "email = %s, airmiles = %d", email_param, atoi(airmiles_param));
                DatabaseResult idDbResult = getIdSaltHashWithEmail(email_param);
                kore_log(2, "test");

                int userId = (int)(get_DatabaseResult(idDbResult, 0, db_user_id));
                kore_log(2, "user id = %d", userId);
                setUserNewAirMiles(userId, atoi(airmiles_param));



                http_response(req, HTTP_STATUS_OK, NULL, NULL);
                return (KORE_RESULT_OK);
            }
        }
    }
    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}
int serve_css(struct http_request *req) {

    http_response_header(req, "content-type", "text/css");
    http_response(req, 200, asset_style_css, asset_len_style_css);

    return (KORE_RESULT_OK);
}

int serve_js(struct http_request *req) {

    http_response_header(req, "content-type", "text/javascript");
    http_response(req, 200, asset_code_js, asset_len_code_js);

    return (KORE_RESULT_OK);
}

int serve_favicon(struct http_request *req) {
    //image/png
    http_response_header(req, "content-type", "image/png");
    http_response(req, 200, asset_favicon_ico, asset_len_favicon_ico);

    return (KORE_RESULT_OK);
}