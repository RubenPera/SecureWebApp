
#include "Header.h"

// Returns userId if a user is logged in
// Returns false if a user is not logged in
int getLoggedInUser(struct http_request *req)
{
    http_populate_cookies(req);

    struct kore_buf *buffer;
    buffer = getSessionCookieValue(req);
    if (buffer != NULL && buffer->data != NULL)
    {
        return getUserIdWithSession(buffer->data);
    }
    return false;
}

int page(struct http_request *req)
{
    int userId = getLoggedInUser(req);
    if (userId)
    {
        struct kore_buf *cookieValue;
        cookieValue = getSessionCookieValue(req);

        update_session(cookieValue->data);

        struct kore_buf *buffer;
        http_populate_cookies(req);
        buffer = kore_buf_alloc(null);

        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);
        kore_buf_replace_string(buffer, "$body$", asset_Index_html, asset_len_Index_html);
        http_response(req, HTTP_STATUS_OK, buffer->data, buffer->offset);
        kore_buf_free(cookieValue);
        return (KORE_RESULT_OK);
    }
    else
    {
        return login(req);
    }
}

// Returns the Login page
int login(struct http_request *req)
{
    char *login_email_param = NULL;
    char *login_password_param = NULL;

    if (getLoggedInUser(req))
    {
        return page(req);
    }

    if (req->method == HTTP_METHOD_POST)
    {
        http_populate_post(req);
        if (KORE_RESULT_OK == http_argument_get_string(req, "email", &login_email_param) &&
            KORE_RESULT_OK == http_argument_get_string(req, "password", &login_password_param))
        {
            DatabaseResult dbResult;
            dbResult = getIdSaltHashWithEmail(login_email_param);

            if (&dbResult != null)
            {
                bool isAuthenticated = login_validate_password(login_password_param,
                                                               get_DatabaseResult(dbResult, 0, 2),
                                                               (unsigned char *)get_DatabaseResult(dbResult, 0, 1));
                if (!isAuthenticated)
                {
                    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
                    return (KORE_RESULT_OK);
                }

                http_populate_cookies(req);
                createSessionCookie(req, (int)get_DatabaseResult(dbResult, 0, db_user_id));

                http_response(req, HTTP_STATUS_OK, NULL, NULL);
                return (KORE_RESULT_OK);
            }
        }
        else
        {
            http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
            return (KORE_RESULT_OK);
        }
    }
    return showLoginPage(req);
}

int flightOverView(struct http_request *req)
{
    int userId = getLoggedInUser(req);
    if (userId)
    {
        struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

        kore_buf_replace_string(buffer, "$body$", asset_FlightOverview_html, asset_len_FlightOverview_html);
        http_response(req, HTTP_STATUS_OK, buffer->data, buffer->offset);
        return (KORE_RESULT_OK);
    }
    else
    {
        return login(req);
    }
}

int getFlights(struct http_request *req)
{
    int userId = getLoggedInUser(req);
    if (userId)
    {
        DatabaseResult newDbResult = getAllFlights();
        if (&newDbResult != null)
        {
            SmartString *str = smart_string_new();
            /*Creating a json object*/
            json_object *container = json_object_new_object();
            /*Creating a json array*/
            json_object *flights = json_object_new_array();
            for (unsigned int row = 0; row < newDbResult.rows; row++)
            {

                json_object *dateValue = json_object_new_string(get_DatabaseResult(newDbResult, row, db_flight_date));

                json_object *priceValue = json_object_new_int(
                    (int)get_DatabaseResult(newDbResult, row, db_flight_price));
                json_object *capacityValue = json_object_new_int(
                    (int)get_DatabaseResult(newDbResult, row, db_flight_capacity));

                json_object *externalIdValue = json_object_new_int(
                    (int)get_DatabaseResult(newDbResult, row, db_flight_external_id));

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

            http_response(req, HTTP_STATUS_OK, str->buffer, (
                                                                unsigned)strlen(str->buffer));

            smart_string_destroy(str);
            return (KORE_RESULT_OK);
        }
    }
    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}

int getLinks(struct http_request *req)
{
    int userId = getLoggedInUser(req);
    if (userId)
    {
        DatabaseResult dbResult = getUserWithId(userId);

        if (&dbResult != null)
        {
            int role = (int)get_DatabaseResult(dbResult, 0, db_user_role);

            SmartString *str = smart_string_new();

            /*Creating a json object*/
            json_object *container = json_object_new_object();
            /*Creating a json array*/

            char *email = get_DatabaseResult(dbResult, 0, db_user_email);
            int miles = (int)get_DatabaseResult(dbResult, 0, db_user_inholland_miles);

            char strMiles[sizeof(miles)];
            sprintf(strMiles, "%d", miles);

            SmartString *userInfo = smart_string_new();
            smart_string_append(userInfo, email);
            smart_string_append(userInfo, " | ");
            smart_string_append(userInfo, strMiles);

            if (role == adminRole)
            {
                // admin
                char *texts[] = {"Flights", "Admin - Users", "Admin - Flights", "User Info", "Logout"};
                char *links[] = {"flightOverView", "adminUsers", "adminFlightOverView", "userInfo", "logout"};
                fillLinks(container, (sizeof(texts) / sizeof(char *)), texts, (sizeof(links) / sizeof(char *)), links);
            }
            else
            {
                // normal user
                char *texts[] = {"Flights", "Logout", userInfo->buffer};
                char *links[] = {"flightOverView", "logout", "userInfo"};
                fillLinks(container, (sizeof(texts) / sizeof(char *)), texts, (sizeof(links) / sizeof(char *)), links);
            }
            kore_log(2, "return value = %s", json_object_to_json_string(container));
            smart_string_append(str, json_object_to_json_string(container));

            http_response_header(req, "content-type", "application/json");

            http_response(req, HTTP_STATUS_OK, str->buffer, (
                                                                unsigned)strlen(str->buffer));

            smart_string_destroy(str);
            return (KORE_RESULT_OK);
        }
    }
    else
    {
        http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
        return (KORE_RESULT_OK);
    }
}

void fillLinks(json_object *container, int sizeTexts, char *texts[sizeTexts], int sizeLinks, char *links[sizeLinks])
{
    json_object *jsonLinks = json_object_new_array();

    for (int i = 0; i < sizeTexts; i++)
    {
        json_object *text = json_object_new_string(texts[i]);
        json_object *link = json_object_new_string(links[i]);

        json_object *item = json_object_new_object();
        json_object_object_add(item, "link", link);
        json_object_object_add(item, "text", text);

        json_object_array_add(jsonLinks, item);
    }

    json_object_object_add(container, "Links", jsonLinks);
}

int bookFlightWithId(struct http_request *req)
{
    int externalId;
    char *strExternalId;
    int userId = getLoggedInUser(req);
    if (userId)
    {
        http_populate_post(req);
        /* Grab it as an actual u_int16_t. */

        if (KORE_RESULT_OK == http_argument_get_string(req, "id", &strExternalId))
        {
            externalId = atoi(strExternalId);

            DatabaseResult flight = getFlightWithExternalId(externalId);
            DatabaseResult user = getUserWithId(userId);
            if (&flight != null && &user != null)
            {
                if ((int)get_DatabaseResult(user, 0, db_user_inholland_miles) >=
                    (int)get_DatabaseResult(flight, 0, db_flight_price))
                {
                    int userId_param = (int)get_DatabaseResult(user, 0, db_user_id),
                        flightId_param = (int)get_DatabaseResult(flight, 0, db_flight_id);
                    createBooking(userId_param, flightId_param);
                    http_response(req, HTTP_STATUS_OK, NULL, NULL);
                    return (KORE_RESULT_OK);
                }
            }
        }
    }
    else
    {
        http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
        return (KORE_RESULT_OK);
    }
}

int adminCancelFlightWithId(struct http_request *req)
{
    int externalId;
    char *strExternalid;

    if (isAdmin(req) && req->method == HTTP_METHOD_POST)
    {
        http_populate_post(req);
        if (KORE_RESULT_OK == http_argument_get_string(req, "id", &strExternalid))
        {
            externalId = atoi(strExternalid);
            DatabaseResult flight = getFlightWithExternalId(externalId);
            if (&flight != null && get_DatabaseResult(flight, 0, db_flight_id) != null)
            {
                cancelFlight((int)get_DatabaseResult(flight, 0, db_flight_id));
                http_response(req, HTTP_STATUS_OK, NULL, NULL);
                return (KORE_RESULT_OK);
            }
        }
    }

    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}

int validate_password_regex(struct http_request *req, char *data)
{
    int result = KORE_RESULT_ERROR;
    pcre *reCompiled;
    pcre_extra *pcreExtra;
    int pcreExecRet;
    int subStrVec[30];
    const char *pcreErrorStr;
    int pcreErrorOffset;
    char *aStrRegex;
    char **aLineToMatch;
    char *testStrings[] = {data};

    kore_log(2, "data = %s", data);
    aStrRegex = "^.*(?=.{12,})(?=.*[a-zA-Z])(?=.*\\d)(?=.*[!#$%&? \"])(?=.*[A-Z]).*$";

    // First, the regex string must be compiled.
    reCompiled = pcre_compile(aStrRegex, 0, &pcreErrorStr, &pcreErrorOffset, NULL);

    if (reCompiled == NULL)
    {
        kore_log(2, "ERROR: Could not compile '%s': %s\n", aStrRegex, pcreErrorStr);
        return result;
    } /* end if */

    // Optimize the regex
    pcreExtra = pcre_study(reCompiled, 0, &pcreErrorStr);

    for (aLineToMatch = testStrings; *aLineToMatch != NULL; aLineToMatch++)
    {
        pcreExecRet = pcre_exec(reCompiled,
                                pcreExtra,
                                *aLineToMatch,
                                strlen(*aLineToMatch), // length of string
                                0,                     // Start looking at this point
                                0,                     // OPTIONS
                                subStrVec,
                                30); // Length of subStrVec
        if (pcreExecRet < 0)
        { // Something bad happened..
            result = (KORE_RESULT_ERROR);
            break;
        }
        else
        {
            result = (KORE_RESULT_OK);
            break;

        } /* end if/else */
        kore_log(2, "optimized");
    }
    kore_log(2, "password is valid %d", result);
    // Free up the regular expression.
    pcre_free(reCompiled);
    return result;
}

int validate_old_password_regex(struct http_request *req, char *data)
{
    return (KORE_RESULT_OK);
}

// Returns the Login page
// Logs out the currenty logged in user by removing the session cookie
int logout(struct http_request *req)
{
    if (getLoggedInUser(req))
    {
        removeSessionCookie(req);
    }
    return showLoginPage(req);
}

int showLoginPage(struct http_request *req)
{
    struct kore_buf *buffer;
    buffer = kore_buf_alloc(asset_len_MasterPage_html);
    kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

    kore_buf_replace_string(buffer, "$body$", asset_Login_html, asset_len_Login_html);

    http_response(req, HTTP_STATUS_OK, buffer->data, buffer->offset);
    return (KORE_RESULT_OK);
}

int userInfo(struct http_request *req)
{

    int userId = getLoggedInUser(req);

    if (userId && req->method == HTTP_METHOD_GET)
    {
        struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

        kore_buf_replace_string(buffer, "$body$", asset_UserInfo_html, asset_len_UserInfo_html);
        http_response(req, HTTP_STATUS_OK, buffer->data, buffer->offset);

        return (KORE_RESULT_OK);
    }
    else
    {
        kore_log(2, "Unauthorized User Access");
        return login(req);
    }
}

int getUserInfo(struct http_request *req)
{
    http_populate_get(req);

    int userId = getLoggedInUser(req);
    DatabaseResult dbResult = getUserWithId(userId);
    if (userId)
    {
        /*Creating a json object*/
        json_object *container = json_object_new_object();
        /*Creating a json array*/
        json_object *users = json_object_new_array();
        json_object *user = json_object_new_object();

        /* Create value object*/
        json_object *emailValue = json_object_new_string(get_DatabaseResult(dbResult, 0, db_user_email));
        json_object *miles = json_object_new_int((int)get_DatabaseResult(dbResult, 0, db_user_inholland_miles));
        /* Create value & name object*/
        json_object_object_add(user, "email", emailValue);
        json_object_object_add(user, "inholland_miles", miles);
        /* Add data to array*/
        json_object_array_add(users, user);
        json_object_object_add(container, "Users", user);

        SmartString *str = smart_string_new();
        smart_string_append(str, json_object_to_json_string(container));

        /*Send data to page - response */
        http_response_header(req, "content-type", "application/json");
        http_response(req, HTTP_STATUS_OK, str->buffer, (unsigned)strlen(str->buffer));

        /*Clean up smartstring - free up memory*/
        smart_string_destroy(str);
    }
    else
    {
        http_response(req, 401, "Unauthorized", (unsigned)strlen("Unauthorized"));
        kore_log(2, "Unauthorized User Access");
        return login(req);
    }

    return (KORE_RESULT_OK);
}

int adminGetUsers(struct http_request *req)
{
    if (isAdmin(req))
    {
        DatabaseResult newDbResult = getAllUsers();
        SmartString *str = smart_string_new();
        /*Creating a json object*/
        json_object *container = json_object_new_object();
        /*Creating a json array*/
        json_object *users = json_object_new_array();
        for (unsigned int row = 0; row < newDbResult.rows; row++)
        {
            json_object *emailValue = json_object_new_string(
                get_DatabaseResult(newDbResult, row, db_user_email));

            json_object *miles = json_object_new_int(
                (int)get_DatabaseResult(newDbResult, row, db_user_inholland_miles));
            json_object *userRole = json_object_new_int(
                (int)get_DatabaseResult(newDbResult, row, db_user_role));
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

        http_response(req, HTTP_STATUS_OK, str->buffer, (
                                                            unsigned)strlen(str->buffer));

        smart_string_destroy(str);
        return (KORE_RESULT_OK);
    }
    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}

int adminUsers(struct http_request *req)
{
    if (isAdmin(req))
    {
        struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

        kore_buf_replace_string(buffer, "$body$", asset_Users_html, asset_len_Users_html);
        http_response(req, HTTP_STATUS_OK, buffer->data, buffer->offset);
        return (KORE_RESULT_OK);
    }
    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}

int adminSetNewAirMilesValue(struct http_request *req)
{
    char *email_param = NULL;
    char *airmiles_param = NULL;

    if (isAdmin(req))
    {
        http_populate_post(req);
        if (KORE_RESULT_OK == http_argument_get_string(req, "email", &email_param) &&
            KORE_RESULT_OK == http_argument_get_string(req, "airmiles", &airmiles_param))
        {

            DatabaseResult idDbResult = getIdSaltHashWithEmail(email_param);

            int userId = (int)(get_DatabaseResult(idDbResult, 0, db_user_id));
            setUserNewAirMiles(userId, atoi(airmiles_param));

            http_response(req, HTTP_STATUS_OK, NULL, NULL);
            return (KORE_RESULT_OK);
        }
    }

    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}

int adminFlightOverView(struct http_request *req)
{
    if (isAdmin(req))
    {
        struct kore_buf *buffer = kore_buf_alloc(asset_len_MasterPage_html);
        kore_buf_append(buffer, asset_MasterPage_html, asset_len_MasterPage_html);

        kore_buf_replace_string(buffer, "$body$", asset_AdminFlightOverview_html, asset_len_AdminFlightOverview_html);
        http_response(req, HTTP_STATUS_OK, buffer->data, buffer->offset);
        return (KORE_RESULT_OK);
    }
    http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
    return (KORE_RESULT_OK);
}

bool isAdmin(struct http_request *req)
{
    int adminId = getLoggedInUser(req);
    if (adminId)
    {
        DatabaseResult dbResult = getUserWithId(adminId);
        if (&dbResult != null)
        {
            int role = (int)get_DatabaseResult(dbResult, 0, db_user_role);
            if (role == adminRole)
            {
                return true;
            }
        }
    }
    return false;
}

int serve_css(struct http_request *req)
{

    http_response_header(req, "content-type", "text/css");
    http_response(req, HTTP_STATUS_OK, asset_style_css, asset_len_style_css);

    return (KORE_RESULT_OK);
}

int serve_js(struct http_request *req)
{

    http_response_header(req, "content-type", "text/javascript");
    http_response(req, HTTP_STATUS_OK, asset_code_js, asset_len_code_js);

    return (KORE_RESULT_OK);
}

int serve_favicon(struct http_request *req)
{
    //image/png
    http_response_header(req, "content-type", "image/png");
    http_response(req, HTTP_STATUS_OK, asset_favicon_ico, asset_len_favicon_ico);

    return (KORE_RESULT_OK);
}

int changePassword(struct http_request *req)
{
    char *old_password = NULL;
    char *new_password = NULL;

    int userId = getLoggedInUser(req);
    if (userId)
    {
        DatabaseResult userdbResult = getUserWithId(userId);
        kore_log(2, "userid = %d", userId);
        http_populate_post(req);

        if (KORE_RESULT_OK == http_argument_get_string(req, "oldpassword", &old_password) &&
            KORE_RESULT_OK == http_argument_get_string(req, "newpassword", &new_password))
        {

            kore_log(2, "received password");
            if (strcmp(old_password, new_password) == 0)
            {
                http_response(req,200,NULL,NULL);
                return (KORE_RESULT_OK);
            }

            DatabaseResult dbResult;
            dbResult = getIdSaltHashWithEmail(get_DatabaseResult(userdbResult, 0, db_user_email));
            bool isAuthenticated = login_validate_password(old_password, get_DatabaseResult(dbResult, 0, 2),
                                                           get_DatabaseResult(dbResult, 0, 1));
            kore_log(2, "old = %s, new = %s", old_password, new_password);
            if (!isAuthenticated)
            {
                kore_log(2, "not authenticated");
                http_response(req, HTTP_STATUS_FORBIDDEN, NULL, NULL);
                return (KORE_RESULT_OK);
            }
            else
            {
                kore_log(2, "is authenticated");
                char hashed_input[STRING_SIZE + 1]; //buffer for the hash function
                hashed_input[STRING_SIZE] = null;
                char salt[STRING_SIZE + 1];
                salt[STRING_SIZE] = null;
                login_generate_salt(STRING_SIZE, salt);
                kore_log(2, "new password = %s", new_password);

                login_hash_password(new_password, salt, LOGIN_HASH_ITERATIONS,
                                    STRING_SIZE / 2,
                                    hashed_input); //hash the password input by the user with the salt in the database
                updateUserPassword(userId, hashed_input, salt);

                http_response(req, HTTP_STATUS_OK, null, 0);
                return (KORE_RESULT_OK);
            }
        }
    }
    else
    {
        http_response(req, 401, "Unauthorized", (unsigned)strlen("Unauthorized"));
        kore_log(2, "Unauthorized User Access");
        return (KORE_RESULT_OK);
    }
    http_response(req, 401, "Unauthorized", (unsigned)strlen("Unauthorized"));
    kore_log(2, "Unauthorized User Access");
    return (KORE_RESULT_OK);
}

int getFlightsBooked(struct http_request *req)
{
    int userId = getLoggedInUser(req);
    if (userId)
    {

        DatabaseResult newDbResult = getAllBookedFlights(userId);
        SmartString *str = smart_string_new();
        /*Creating a json object*/
        json_object *container = json_object_new_object();
        /*Creating a json array*/
        json_object *flights = json_object_new_array();
        for (unsigned int row = 0; row < newDbResult.rows; row++)
        {

            json_object *dateValue = json_object_new_string(get_DatabaseResult(newDbResult, row, db_flight_date));

            json_object *priceValue = json_object_new_int((int)get_DatabaseResult(newDbResult, row, db_flight_price));
            json_object *capacityValue = json_object_new_int(
                (int)get_DatabaseResult(newDbResult, row, db_flight_capacity));

            json_object *externalIdValue = json_object_new_int(
                (int)get_DatabaseResult(newDbResult, row, db_flight_external_id));

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

        http_response(req, HTTP_STATUS_OK, str->buffer, (
                                                            unsigned)strlen(str->buffer));

        smart_string_destroy(str);
        return (KORE_RESULT_OK);
    }
    else
    {
        http_response(req, 401, "Unauthorized", (unsigned)strlen("Unauthorized"));
        kore_log(2, "Unauthorized User Access");
        return login(req);
    }
}
