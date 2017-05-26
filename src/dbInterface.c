#include "Header.h"

#define get_user_id_salt_hash_with_email 'select id,pasword_hash, pasword_salt from user where email = ?;';

void _dbConnect(MYSQL *conn) {
    char *server = "127.0.0.1";
    char *user = "root";
    char *password = "TeamAlfa1!"; /* set me first */
    char *database = "secure_web_app_database";
    unsigned int port = 3306;

    /* Connect to database */
    if (!mysql_real_connect(conn, server,
                            user, password, database, port, NULL, 0)) {
        kore_log(2, mysql_error(conn));
    }
    mysql_select_db(conn, database);
}

void _dbDisconnect(MYSQL *conn) {
    mysql_close(conn);
}

DatabaseResult getUsers() {
    MYSQL_ROW row;
    MYSQL *conn;
    int _row = 0,
            columnCounter = 0;
    DatabaseResult dbResult;

    conn = mysql_init(NULL);
    _dbConnect(conn);

    mysql_query(conn, "call get_all_users()");

    MYSQL_RES *result = mysql_store_result(conn);
    if (result != NULL) {
        unsigned int num_fields = mysql_num_fields(result);

        dbResult = init_DatabaseResult(result->row_count, num_fields);

        while ((row = mysql_fetch_row(result))) {
            for (columnCounter = 0; columnCounter < num_fields; columnCounter++) {
                dbResult.data[_row][columnCounter] = row[columnCounter];
            }
            _row++;
        }
        for (int i = 0; i < dbResult.rows; i++) {
            for (int y = 0; y < dbResult.columns; y++) {
                kore_log(2, dbResult.data[i][y]);
            }
        }
    }

    _dbDisconnect(conn);
    return dbResult;
}

// Updates the session last_updated variable
// Session is deleted when older than 15 minutes
void update_session(char *session_id) {
    kore_log(1, "update session");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    unsigned long str_length = STRING_SIZE;
    char sessionId_param[STRING_SIZE];

    conn = mysql_init(NULL);
    _dbConnect(conn);
    char *query = "call update_session_last_use(?)";

    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        kore_log(1, "error");
    }

    memset(bind, 0, sizeof(bind));

    strncpy(sessionId_param, session_id, STRING_SIZE); /* string  */
    str_length = strlen(sessionId_param);

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *) sessionId_param;
    bind[0].buffer_length = STRING_SIZE;
    bind[0].is_null = 0;
    bind[0].length = &str_length;


    if (mysql_stmt_bind_param(stmt, bind)) {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt)) {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt)) {
        kore_log(2, "ERROR executing");
    }
    mysql_stmt_close(stmt);
    _dbDisconnect(conn);
    // https://dev.mysql.com/doc/refman/5.6/en/mysql-stmt-execute.html
}

DatabaseResult getIdSaltHashWithEmail(char *email) {
    kore_log(1, " getIdSaltHashWithEmail");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND inputBind[1];
    MYSQL_BIND outputBind[3];
    unsigned long str_length = STRING_SIZE;

    char email_param[STRING_SIZE];
    char salt_param[STRING_SIZE + 1];
    salt_param[STRING_SIZE] = NULL;

    char hash_param[STRING_SIZE + 1];
    hash_param[STRING_SIZE] = NULL;


    unsigned long length[3] = {0, 0, 0};
    int userId = 0;
    unsigned int i = 0;

    // MySQL bool to make pointers fully compatible
    my_bool is_null[3] = {false, false, false};
    my_bool error[3] = {false, false, false};

    conn = mysql_init(NULL);
    _dbConnect(conn);

    char *query = "call get_user_salt_hash_with_email(?);";

    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        kore_log(1, "error");
    }

    memset(inputBind, 0, sizeof(inputBind));

    strncpy(email_param, email, STRING_SIZE); /* string  */
    str_length = strlen(email_param);

    i = 0;
    inputBind[i].buffer_type = MYSQL_TYPE_STRING;
    inputBind[i].buffer = (char *) email_param;
    inputBind[i].buffer_length = STRING_SIZE;
    inputBind[i].is_null = 0;
    inputBind[i].length = &str_length;


    if (mysql_stmt_bind_param(stmt, inputBind)) {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt)) {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt)) {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));

    i = 0;
    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
    outputBind[i].buffer = (char *) &userId;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
    outputBind[i].buffer = (char *) salt_param;
    outputBind[i].buffer_length = STRING_SIZE;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
    outputBind[i].buffer = (char *) hash_param;
    outputBind[i].buffer_length = STRING_SIZE;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind)) {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt)) {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    mysql_stmt_fetch(stmt);
    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult(1, 3);
//    kore_log(2, "hash_param = %s", hash_param);
    i = 0;

    if (!is_null[i]) {
        set_DatabaseResult(dbResult, 0, i, (char *) userId);
    } else {
        set_DatabaseResult(dbResult, 0, i, NULL);
    }

    i++;
    if (!is_null[i]) {
        set_DatabaseResult(dbResult, 0, i, salt_param);
    } else {
        set_DatabaseResult(dbResult, 0, i, NULL);
    }
    i++;
    if (!is_null[i]) {
        set_DatabaseResult(dbResult, 0, i, hash_param);
    } else {
        set_DatabaseResult(dbResult, 0, i, NULL);
    }
    return dbResult;
}

void sqlToJson(SmartString *str, char *query, char *groupname) {
    MYSQL_FIELD *field;
    MYSQL_ROW row;
    MYSQL *conn;
    int col_counter = 0;

    /*Creating a json object*/
    json_object *container = json_object_new_object();

    conn = mysql_init(NULL);
    _dbConnect(conn);

    mysql_query(conn, query);

    MYSQL_RES *result = mysql_store_result(conn);
    if (result != NULL) {

        int num_fields = mysql_num_fields(result);
        char **field_array;
        field_array = (char **) malloc(sizeof(char *) * (num_fields + 1));
        for (int i = 0; i < num_fields; i++) {
            field = mysql_fetch_field(result);
            field_array[i] = field->name;
        }

        /*Creating a json array*/
        json_object *items = json_object_new_array();

        while ((row = mysql_fetch_row(result))) {
            //Reading a row.
            json_object *item = json_object_new_object();
            for (col_counter = 0; col_counter < num_fields; col_counter++) {
                //Retrieving a col.
                char *col_value = row[col_counter];
                //Retrieving the name of the field belonging to this row.
                char *field_name = field_array[col_counter];
                /*Creating a json object*/

                /*Creating a json string*/
                json_object *field_value = json_object_new_string(col_value);
                /*Adding the string to the object*/
                json_object_object_add(item, field_name, field_value);
            }
            json_object_array_add(items, item);
        }
        /*Adding the json array to the object*/
        json_object_object_add(container, groupname, items);
    }

    _dbDisconnect(conn);
    smart_string_append(str, json_object_to_json_string(container));
}

void DoHet(SmartString *str) {
    smart_string_append(str, "hallo");
}

void createBooking(char *userId, char *flightId) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW userRow;
    MYSQL_ROW flightRow;
    int *flightPrice = 0;
    int *userAirmiles = 0;
    SmartString *updateFlight = smart_string_new();
    SmartString *updateBooking = smart_string_new();
    SmartString *getFlightPriceQuery = smart_string_new();
    SmartString *getUserAirmilesQuery = smart_string_new();
    SmartString *updateUserAirmilesQuery = smart_string_new();

    /*Connect to db*/
    conn = mysql_init(NULL);
    _dbConnect(conn);

    /*Create query for getting airmiles by userId*/
    createGetUserAirmilesQuery(getUserAirmilesQuery, userId);
    createGetFlightPriceQuery(getFlightPriceQuery, flightId);

    mysql_query(conn, getUserAirmilesQuery->buffer);
    result = mysql_store_result(conn);
    if (result != NULL) {
        kore_log(2, getUserAirmilesQuery->buffer);
        userRow = mysql_fetch_row(result);
        userAirmiles = atoi(userRow[0]);
        kore_log(2, userRow[0]);
        mysql_free_result(result);
        result = NULL;
        mysql_next_result(conn);
    } else {
        kore_log(2, "Error: ResultNullexception");
    }

    mysql_query(conn, getFlightPriceQuery->buffer);
    result = mysql_store_result(conn);

    if (result != NULL) {
        kore_log(2, getFlightPriceQuery->buffer);
        flightRow = mysql_fetch_row(result);
        flightPrice = atoi(flightRow[0]);
        kore_log(2, flightRow[0]);
        result = NULL;
        mysql_next_result(conn);
    } else {
        kore_log(2, "Error: ResultNullexception");
    }

    if (userAirmiles > flightPrice) {
        /*Create a query for decreasing capacity of flight by 1 */
        createUpdateFlightCapacityQuery(updateFlight, flightId);
        kore_log(2, updateFlight->buffer);
        if (mysql_query(conn, updateFlight->buffer) != 0) {
            kore_log(2, "UPDATE Flight failed: Stopping booking..");
        } else {

            /*Create a qeury for inserting a booking into the booking table */
            createInsertBookingQuery(updateBooking, userId, flightId);
            kore_log(2, updateBooking->buffer);
            if (mysql_query(conn, updateBooking->buffer) != 0) {
                kore_log(2, "INSERT INTO Booking failed: Stopping booking..");
            } else {

                createUpdateUserAirMilesQuery(updateUserAirmilesQuery, userId, flightPrice);
                kore_log(2, updateUserAirmilesQuery->buffer);
                if (mysql_query(conn, updateUserAirmilesQuery->buffer) != 0) {
                    kore_log(2, "UPDATE User failed: Stopping booking..");
                } else {
                    kore_log(2, "Booking transaction succeeded.");
                }
            }
        }
    }

    /*Disconnect from db*/
    _dbDisconnect(conn);

    /*Destroy/ Clean up smartstrings*/
    smart_string_destroy(updateFlight);
    smart_string_destroy(updateBooking);
    smart_string_destroy(getFlightPriceQuery);
    smart_string_destroy(getUserAirmilesQuery);
    smart_string_destroy(updateUserAirmilesQuery);
}

void getUserAirmiles(SmartString *output, char *userId) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    SmartString *getUserAirmilesQuery = smart_string_new();

    /*connect to db*/
    conn = mysql_init(NULL);
    _dbConnect(conn);

    createGetUserAirmilesQuery(getUserAirmilesQuery, userId);

    mysql_query(conn, getUserAirmilesQuery->buffer);

    result = mysql_store_result(conn);
    if (result != NULL) {
        row = mysql_fetch_row(result);
        smart_string_append(output, row[0]);
    }
    _dbDisconnect(conn);
}

void createInsertBookingQuery(SmartString *str, char *userId, char *flightId) {
    smart_string_append(str, "call insert_booking(");
    smart_string_append(str, userId);
    smart_string_append(str, ",");
    smart_string_append(str, flightId);
    smart_string_append(str, ");");
}

void createUpdateFlightCapacityQuery(SmartString *str, char *flightId) {
    smart_string_append(str, "call update_flight_capacity(");
    smart_string_append(str, flightId);
    smart_string_append(str, ");");
}

void createGetFlightPriceQuery(SmartString *str, char *flightId) {
    smart_string_append(str, "call get_flight_price(");
    smart_string_append(str, flightId);
    smart_string_append(str, ");");
}

void createGetUserAirmilesQuery(SmartString *str, char *userId) {
    smart_string_append(str, "call get_user_airmiles_by_userid(");
    smart_string_append(str, userId);
    smart_string_append(str, ");");
}

void createUpdateUserAirMilesQuery(SmartString *str, char *userId, int price) {
    smart_string_append(str, "call update_user_airmiles(");
    smart_string_append(str, userId);
    smart_string_append(str, ",");
    smart_string_append_sprintf(str, "%d", price);
    smart_string_append(str, ");");
}

void createSessionRow(int userId, char *sessionId) {
    kore_log(1, "createSessionRow");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];

    unsigned long str_length = STRING_SIZE;

    char session_id_param[STRING_SIZE];

    conn = mysql_init(NULL);
    _dbConnect(conn);
    char *query = "call create_session_row(?,?)";

    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        kore_log(1, "error");
    }

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *) &userId;
    bind[0].is_null = 0;
    bind[0].length = 0;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char *) session_id_param;
    bind[1].buffer_length = STRING_SIZE;
    bind[1].is_null = 0;
    bind[1].length = &str_length;

    strncpy(session_id_param, sessionId, STRING_SIZE); /* string  */
    str_length = strlen(session_id_param);

    if (mysql_stmt_bind_param(stmt, bind)) {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt)) {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt)) {
        kore_log(2, "ERROR executing");
    }
    mysql_stmt_close(stmt);
    _dbDisconnect(conn);
}

// Returns NULL if no user can be found
// Returns userId if a user is logged in
int getUserIdWithSession(char sessionId[STRING_SIZE - 1]) {
    kore_log(1, " getUserWithSession");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND inputBind[1];
    MYSQL_BIND outputBind[1];
    unsigned long str_length = STRING_SIZE;

    char session_id_param[STRING_SIZE];
    int userId = 0;

    unsigned long length[1];
    // MySQL bool to make pointers fully compatible
    my_bool is_null[1];
    my_bool error[1];

    conn = mysql_init(NULL);
    _dbConnect(conn);

    char *query = "call get_user_id_from_session(?);";

    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        kore_log(1, "error");
    }

    memset(inputBind, 0, sizeof(inputBind));

    inputBind[0].buffer_type = MYSQL_TYPE_STRING;
    inputBind[0].buffer = (char *) session_id_param;
    inputBind[0].buffer_length = STRING_SIZE;
    inputBind[0].is_null = 0;
    inputBind[0].length = &str_length;

    strncpy(session_id_param, sessionId, STRING_SIZE); /* string  */

    str_length = strlen(session_id_param);

    if (mysql_stmt_bind_param(stmt, inputBind)) {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt)) {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt)) {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));
    outputBind[0].buffer_type = MYSQL_TYPE_LONG;
    outputBind[0].buffer = (char *) &userId;
    outputBind[0].is_null = &is_null[0];
    outputBind[0].length = &length[0];
    outputBind[0].error = &error[0];


    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind)) {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt)) {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    mysql_stmt_fetch(stmt);
    _dbDisconnect(conn);
    if (!is_null[0]) {
        return userId;
    }
    return 0;
}