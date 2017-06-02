#include "Header.h"

#define get_user_id_salt_hash_with_email 'select id,pasword_hash, pasword_salt from user where email = ?;';

void _dbConnect(MYSQL *conn)
{
    char *server = "127.0.0.1";
    char *user = "root";
    char *password = "TeamAlfa1!"; /* set me first */
    char *database = "secure_web_app_database";
    unsigned int port = 3306;

    /* Connect to database */
    if (!mysql_real_connect(conn, server,
                            user, password, database, port, NULL, 0))
    {
        kore_log(2, mysql_error(conn));
    }
    mysql_select_db(conn, database);
}

void _dbDisconnect(MYSQL *conn)
{
    mysql_close(conn);
}

// Updates the session last_updated variable
// Session is deleted when older than 15 minutes
void update_session(char *session_id)
{
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
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(bind, 0, sizeof(bind));

    strncpy(sessionId_param, session_id, STRING_SIZE); /* string  */
    str_length = strlen(sessionId_param);

    fillInputBindString(bind, 0, &sessionId_param, &str_length);

    if (mysql_stmt_bind_param(stmt, bind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
    }
    mysql_stmt_close(stmt);
    _dbDisconnect(conn);
    // https://dev.mysql.com/doc/refman/5.6/en/mysql-stmt-execute.html
}

DatabaseResult getIdSaltHashWithEmail(char *email)
{
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
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(inputBind, 0, sizeof(inputBind));

    strncpy(email_param, email, STRING_SIZE); /* string  */
    str_length = strlen(email_param);

    i = 0;
    fillInputBindString(inputBind, i, &email_param, &str_length);

    if (mysql_stmt_bind_param(stmt, inputBind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));

    i = 0;
    fillOutputBindLong(outputBind, i++, &userId, is_null, length, error);
    fillOutputBindString(outputBind, i++, &salt_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &hash_param, is_null, length, error);

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind))
    {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    mysql_stmt_fetch(stmt);
    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult(1, i);
    i = 0;


    set_DatabaseResult(dbResult, 0, i++, (char *)userId);
    SmartString * strSalt_param = smart_string_new();
    smart_string_append(strSalt_param, salt_param);
    set_DatabaseResult(dbResult, 0, i++, strSalt_param->buffer);
    SmartString * strHash_param = smart_string_new();
    smart_string_append(strHash_param, hash_param);
    set_DatabaseResult(dbResult, 0, i++, strHash_param->buffer);

    return dbResult;
}

DatabaseResult getAllFlights()
{
    kore_log(1, " getAllFlights");
    MYSQL *conn;
    MYSQL_STMT *stmt;

    unsigned int sizeOfOutPutBind = 7;

    unsigned int i = 0;

    MYSQL_BIND outputBind[sizeOfOutPutBind];

    int flightId_parm = 0,
        price_param = 0,
        capacity_param = 0,
        external_id_parm = 0;
    MYSQL_TIME date;

    char flight_source[STRING_SIZE + 1];
    flight_source[STRING_SIZE] = NULL;

    char flight_destination[STRING_SIZE + 1];
    flight_destination[STRING_SIZE] = NULL;

    // MySQL bool to make pointers fully compatible
    my_bool is_null[sizeOfOutPutBind];
    my_bool error[sizeOfOutPutBind];
    unsigned long length[sizeOfOutPutBind];

    conn = mysql_init(NULL);
    _dbConnect(conn);

    char *query = "call get_all_flights();";
    //    char *query = "get * from flight;";
    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));

    i = 0;
    fillOutputBindLong(outputBind, i++, &flightId_parm, is_null, length, error);
    fillOutputBindDate(outputBind, i++, &date, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &price_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &flight_source, is_null, length, error);
    fillOutputBindString(outputBind, i++, &flight_destination, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &capacity_param, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &external_id_parm, is_null, length, error);

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind))
    {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult((unsigned int)mysql_stmt_num_rows(stmt), i);
    unsigned int y = 0;
    while (!mysql_stmt_fetch(stmt))
    {
        set_DatabaseResult(dbResult, y, db_flight_id, (char *)flightId_parm);

        SmartString *strDate = smart_string_new();

        char formattedDate[20];
        sprintf(formattedDate, "%02d-%02d-%04d", date.day, date.month, date.year);

        smart_string_append(strDate, formattedDate);
        set_DatabaseResult(dbResult, y, db_flight_date, strDate->buffer);

        set_DatabaseResult(dbResult, y, db_flight_price, (char *)price_param);

        SmartString *strFlight_source = smart_string_new();
        smart_string_append(strFlight_source, flight_source);
        set_DatabaseResult(dbResult, y, db_flight_flight_source, strFlight_source->buffer);

        SmartString *strFlight_destination = smart_string_new();
        smart_string_append(strFlight_destination, flight_destination);
        set_DatabaseResult(dbResult, y, db_flight_flight_destination, strFlight_destination->buffer);

        set_DatabaseResult(dbResult, y, db_flight_capacity, (char *)capacity_param);

        set_DatabaseResult(dbResult, y, db_flight_external_id, (char *)external_id_parm);
        y++;
    }

    return dbResult;
}

void sqlToJson(SmartString *str, char *query, char *groupname)
{
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
    if (result != NULL)
    {

        int num_fields = mysql_num_fields(result);
        char **field_array;
        field_array = (char **)malloc(sizeof(char *) * (num_fields + 1));
        for (int i = 0; i < num_fields; i++)
        {
            field = mysql_fetch_field(result);
            field_array[i] = field->name;
        }

        /*Creating a json array*/
        json_object *items = json_object_new_array();

        while ((row = mysql_fetch_row(result)))
        {
            //Reading a row.
            json_object *item = json_object_new_object();
            for (col_counter = 0; col_counter < num_fields; col_counter++)
            {
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

void DoHet(SmartString *str)
{
    smart_string_append(str, "hallo");
}

void createBooking(int userId, int flightId)
{
    kore_log(1, "createBooking");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];

    unsigned int i = 0;

    conn = mysql_init(NULL);
    _dbConnect(conn);
    char *query = "call create_booking_and_update_money(?,?)";

    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(bind, 0, sizeof(bind));

    fillInputBindLong(bind, i++, &userId);
    fillInputBindLong(bind, i++, &flightId);

    if (mysql_stmt_bind_param(stmt, bind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {

        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    mysql_stmt_close(stmt);
    _dbDisconnect(conn);
}

void cancelFlight(int flightId)
{
    kore_log(1, "cancelFlight");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];

    unsigned int i = 0;

    conn = mysql_init(NULL);
    _dbConnect(conn);
    char *query = "call cancel_booking(?)";

    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(bind, 0, sizeof(bind));

    fillInputBindLong(bind, i++, &flightId);

    if (mysql_stmt_bind_param(stmt, bind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {

        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    mysql_stmt_close(stmt);
    _dbDisconnect(conn);
}

// void getUserAirmiles(SmartString *output, int userId)
// {
//     MYSQL *conn;
//     MYSQL_RES *result;
//     MYSQL_ROW row;
//     SmartString *getUserAirmilesQuery = smart_string_new();

//     /*connect to db*/
//     conn = mysql_init(NULL);
//     _dbConnect(conn);

//     createGetUserAirmilesQuery(getUserAirmilesQuery, userId);

//     mysql_query(conn, getUserAirmilesQuery->buffer);

//     result = mysql_store_result(conn);
//     if (result != NULL)
//     {
//         row = mysql_fetch_row(result);
//         smart_string_append(output, row[0]);
//     }
//     _dbDisconnect(conn);
// }

// void createInsertBookingQuery(SmartString *str, int userId, char *flightId)
// {
//     smart_string_append(str, "call insert_booking(");
//     smart_string_append_sprintf(str, "%d", userId);
//     smart_string_append(str, ",");
//     smart_string_append_sprintf(str, "%d", flightId);
//     smart_string_append(str, ");");
// }

// void createUpdateFlightCapacityQuery(SmartString *str, int flightId)
// {
//     smart_string_append(str, "call update_flight_capacity(");
//     smart_string_append_sprintf(str, "%d", flightId);
//     smart_string_append(str, ");");
// }

// void createGetFlightPriceQuery(SmartString *str, int flightId)
// {
//     smart_string_append(str, "call get_flight_price(");
//     kore_log(2, "flightId = %d", flightId);
//     smart_string_append_sprintf(str, "%d", flightId);
//     smart_string_append(str, ");");
// }

// void createGetUserAirmilesQuery(SmartString *str, int userId)
// {
//     smart_string_append(str, "call get_user_airmiles_by_userid(");
//     smart_string_append_sprintf(str, "%d", userId);
//     smart_string_append(str, ");");
// }

// void createUpdateUserAirMilesQuery(SmartString *str, int userId, int price)
// {
//     smart_string_append(str, "call update_user_airmiles(");
//     smart_string_append_sprintf(str, "%d", userId);
//     smart_string_append(str, ",");
//     smart_string_append_sprintf(str, "%d", price);
//     smart_string_append(str, ");");
// }

void createSessionRow(int userId, char *sessionId)
{
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
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(bind, 0, sizeof(bind));
    unsigned int i = 0;
    fillInputBindLong(bind, i++, &userId);
    fillInputBindString(bind, i++, &session_id_param, &str_length);

    strncpy(session_id_param, sessionId, STRING_SIZE); /* string  */
    str_length = strlen(session_id_param);

    if (mysql_stmt_bind_param(stmt, bind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
    }
    mysql_stmt_close(stmt);
    _dbDisconnect(conn);
}

// Returns NULL if no user can be found
// Returns userId if a user is logged in
int getUserIdWithSession(char sessionId[STRING_SIZE - 1])
{
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
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(inputBind, 0, sizeof(inputBind));
    fillInputBindString(inputBind, 0, &session_id_param, &str_length);

    strncpy(session_id_param, sessionId, STRING_SIZE); /* string  */

    str_length = strlen(session_id_param);

    if (mysql_stmt_bind_param(stmt, inputBind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));
    fillOutputBindLong(outputBind, 0, &userId, is_null, length, error);

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind))
    {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    mysql_stmt_fetch(stmt);
    kore_log(2, "returning %d", userId);
    _dbDisconnect(conn);
    if (!is_null[0])
    {
        return userId;
    }
    return 0;
}

DatabaseResult getUserWithId(int userId)
{
    kore_log(1, "getUserWithId");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND inputBind[1];
    MYSQL_BIND outputBind[6];

    unsigned long length[6];
    // MySQL bool to make pointers fully compatible
    my_bool is_null[6];
    my_bool error[6];

    unsigned long str_length = STRING_SIZE;

    char email_param[STRING_SIZE];
    char salt_param[STRING_SIZE + 1];
    salt_param[STRING_SIZE] = NULL;

    char hash_param[STRING_SIZE + 1];
    hash_param[STRING_SIZE] = NULL;

    int id_param = 0,
        miles_param = 0,
        role_param = 0;

    conn = mysql_init(NULL);
    _dbConnect(conn);
    char *query = "call get_user_with_id(?)";

    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(inputBind, 0, sizeof(inputBind));

    fillInputBindLong(inputBind, 0, &userId);

    if (mysql_stmt_bind_param(stmt, inputBind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));

    unsigned int i = 0;
    fillOutputBindLong(outputBind, i++, &id_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &salt_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &hash_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &email_param, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &miles_param, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &role_param, is_null, length, error);

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind))
    {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    mysql_stmt_fetch(stmt);
    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult(1, i);

    i = 0;
    set_DatabaseResult(dbResult, 0, i++, (char *)id_param);
    set_DatabaseResult(dbResult, 0, i++, hash_param);
    set_DatabaseResult(dbResult, 0, i++, salt_param);
    set_DatabaseResult(dbResult, 0, i++, email_param);
    set_DatabaseResult(dbResult, 0, i++, (char *)miles_param);
    set_DatabaseResult(dbResult, 0, i++, (char *)role_param);

    return dbResult;
}

DatabaseResult getFlightWithId(int flightId)
{
    kore_log(1, " getFlightWithId");
    MYSQL *conn;
    MYSQL_STMT *stmt;

    unsigned int sizeOfOutPutBind = 7;

    unsigned int i = 0;

    MYSQL_BIND outputBind[sizeOfOutPutBind];
    MYSQL_BIND inputBind[1];

    int flightId_param = 0,
        price_param = 0,
        capacity_param = 0,
        external_id_param = 0;
    MYSQL_TIME date;

    char flight_source[STRING_SIZE + 1];
    flight_source[STRING_SIZE] = NULL;

    char flight_destination[STRING_SIZE + 1];
    flight_destination[STRING_SIZE] = NULL;

    // MySQL bool to make pointers fully compatible
    my_bool is_null[sizeOfOutPutBind];
    my_bool error[sizeOfOutPutBind];
    unsigned long length[sizeOfOutPutBind];

    conn = mysql_init(NULL);
    _dbConnect(conn);

    char *query = "call get_all_flights(?);";

    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(inputBind, 0, sizeof(inputBind));

    fillInputBindLong(inputBind, 0, &flightId);

    if (mysql_stmt_bind_param(stmt, inputBind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));

    i = 0;
    fillOutputBindLong(outputBind, i++, &flightId_param, is_null, length, error);
    fillOutputBindDate(outputBind, i++, &date, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &price_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &flight_source, is_null, length, error);
    fillOutputBindString(outputBind, i++, &flight_destination, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &capacity_param, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &external_id_param, is_null, length, error);

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind))
    {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult((unsigned int)mysql_stmt_num_rows(stmt), i);
    unsigned int y = 0;
    while (!mysql_stmt_fetch(stmt))
    {
        set_DatabaseResult(dbResult, y, db_flight_id, (char *)flightId_param);

        SmartString *strDate = smart_string_new();

        char formattedDate[20];
        sprintf(formattedDate, "%02d-%02d-%04d", date.day, date.month, date.year);

        smart_string_append(strDate, formattedDate);
        set_DatabaseResult(dbResult, y, db_flight_date, strDate->buffer);

        set_DatabaseResult(dbResult, y, db_flight_price, (char *)price_param);

        SmartString *strFlight_source = smart_string_new();
        smart_string_append(strFlight_source, flight_source);
        set_DatabaseResult(dbResult, y, db_flight_flight_source, strFlight_source->buffer);

        SmartString *strFlight_destination = smart_string_new();
        smart_string_append(strFlight_destination, flight_destination);
        set_DatabaseResult(dbResult, y, db_flight_flight_destination, strFlight_destination->buffer);

        set_DatabaseResult(dbResult, y, db_flight_capacity, (char *)capacity_param);

        set_DatabaseResult(dbResult, y, db_flight_external_id, (char *)external_id_param);
        y++;
    }

    return dbResult;
}

DatabaseResult getFlightWithExternalId(int externalId)
{
    kore_log(1, " getFlightWithExternalId");
    MYSQL *conn;
    MYSQL_STMT *stmt;

    unsigned int sizeOfOutPutBind = 7;

    unsigned int i = 0;

    MYSQL_BIND outputBind[sizeOfOutPutBind];
    MYSQL_BIND inputBind[1];

    int flightId_param = 0,
        price_param = 0,
        capacity_param = 0,
        external_id_param = 0;
    MYSQL_TIME date;

    char flight_source[STRING_SIZE + 1];
    flight_source[STRING_SIZE] = NULL;

    char flight_destination[STRING_SIZE + 1];
    flight_destination[STRING_SIZE] = NULL;

    // MySQL bool to make pointers fully compatible
    my_bool is_null[sizeOfOutPutBind];
    my_bool error[sizeOfOutPutBind];
    unsigned long length[sizeOfOutPutBind];

    conn = mysql_init(NULL);
    _dbConnect(conn);

    char *query = "call get_all_flights_with_external_id(?);";

    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(inputBind, 0, sizeof(inputBind));

    fillInputBindLong(inputBind, 0, &externalId);

    if (mysql_stmt_bind_param(stmt, inputBind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));

    i = 0;
    fillOutputBindLong(outputBind, i++, &flightId_param, is_null, length, error);
    fillOutputBindDate(outputBind, i++, &date, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &price_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &flight_source, is_null, length, error);
    fillOutputBindString(outputBind, i++, &flight_destination, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &capacity_param, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &external_id_param, is_null, length, error);

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind))
    {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult((unsigned int)mysql_stmt_num_rows(stmt), i);
    unsigned int y = 0;
    while (!mysql_stmt_fetch(stmt))
    {
        set_DatabaseResult(dbResult, y, db_flight_id, (char *)flightId_param);

        SmartString *strDate = smart_string_new();

        char formattedDate[20];
        sprintf(formattedDate, "%02d-%02d-%04d", date.day, date.month, date.year);

        smart_string_append(strDate, formattedDate);
        set_DatabaseResult(dbResult, y, db_flight_date, strDate->buffer);

        set_DatabaseResult(dbResult, y, db_flight_price, (char *)price_param);

        SmartString *strFlight_source = smart_string_new();
        smart_string_append(strFlight_source, flight_source);
        set_DatabaseResult(dbResult, y, db_flight_flight_source, strFlight_source->buffer);

        SmartString *strFlight_destination = smart_string_new();
        smart_string_append(strFlight_destination, flight_destination);
        set_DatabaseResult(dbResult, y, db_flight_flight_destination, strFlight_destination->buffer);

        set_DatabaseResult(dbResult, y, db_flight_capacity, (char *)capacity_param);

        set_DatabaseResult(dbResult, y, db_flight_external_id, (char *)external_id_param);
        y++;
    }

    return dbResult;
}

DatabaseResult getAllUsers()
{
    kore_log(1, "getAllUsers");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND outputBind[6];

    unsigned long length[6];
    // MySQL bool to make pointers fully compatible
    my_bool is_null[6];
    my_bool error[6];

    char email_param[STRING_SIZE + 1];
    email_param[STRING_SIZE] = NULL;
    char salt_param[STRING_SIZE + 1];
    salt_param[STRING_SIZE] = NULL;

    char hash_param[STRING_SIZE + 1];
    hash_param[STRING_SIZE] = NULL;

    int id_param = 0,
        miles_param = 0,
        role_param = 0;

    conn = mysql_init(NULL);
    _dbConnect(conn);
    char *query = "call get_all_users()";

    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));

    unsigned int i = 0;
    fillOutputBindLong(outputBind, i++, &id_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &salt_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &hash_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &email_param, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &miles_param, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &role_param, is_null, length, error);

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind))
    {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult(mysql_stmt_num_rows(stmt), i);
    unsigned int y = 0;
    while (!mysql_stmt_fetch(stmt))
    {

        i = 0;
        set_DatabaseResult(dbResult, y, db_user_id, (char *)id_param);

        SmartString *strHash_param = smart_string_new();
        smart_string_append(strHash_param, hash_param);
        set_DatabaseResult(dbResult, y, db_user_pasword_hash, strHash_param->buffer);

        SmartString *strSalt_param = smart_string_new();
        smart_string_append(strSalt_param, salt_param);
        set_DatabaseResult(dbResult, y, db_user_pasword_salt, strSalt_param);

        SmartString *strEmail_param = smart_string_new();
        smart_string_append(strEmail_param, email_param);
        set_DatabaseResult(dbResult, y, db_user_email, strEmail_param->buffer);

        set_DatabaseResult(dbResult, y, db_user_inholland_miles, (char *)miles_param);
        set_DatabaseResult(dbResult, y, db_user_role, (char *)role_param);
        y++;
    }

    return dbResult;
}

void setUserNewAirMiles(int userId, int airMiles)
{

    kore_log(1, "setUserNewAirMiles");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND inputBind[2];
    unsigned int i = 0;
    conn = mysql_init(NULL);
    _dbConnect(conn);
    char *query = "call set_airMiles_for_userId(?,?)";

    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(inputBind, 0, sizeof(inputBind));

    i = 0;
    fillInputBindLong(inputBind, i++, &userId);
    fillInputBindLong(inputBind, i++, &airMiles);

    if (mysql_stmt_bind_param(stmt, inputBind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
}

void fillOutputBindLong(MYSQL_BIND *bind, unsigned int i, int *param, my_bool *is_null, unsigned long *length,
                        my_bool *error)
{
    bind[i].buffer_type = MYSQL_TYPE_LONG;
    bind[i].buffer = (char *)param;
    bind[i].is_null = &is_null[i];
    bind[i].length = &length[i];
    bind[i].error = &error[i];
}

void fillOutputBindString(MYSQL_BIND *bind, unsigned int i, char **param, my_bool *is_null, unsigned long *length,
                          my_bool *error)
{
    bind[i].buffer_type = MYSQL_TYPE_STRING;
    bind[i].buffer = (char *)param;
    bind[i].buffer_length = STRING_SIZE;
    bind[i].is_null = &is_null[i];
    bind[i].length = &length[i];
    bind[i].error = &error[i];
}

void fillOutputBindDate(MYSQL_BIND *bind, unsigned int i, MYSQL_TIME *param, my_bool *is_null, unsigned long *length,
                        my_bool *error)
{
    bind[i].buffer_type = MYSQL_TYPE_DATETIME;
    bind[i].buffer = (char *)param;
    bind[i].buffer_length = STRING_SIZE;
    bind[i].is_null = &is_null[i];
    bind[i].length = &length[i];
    bind[i].error = &error[i];
}

void fillInputBindString(MYSQL_BIND *bind, unsigned int i, char **param, unsigned long *length)
{

    bind[i].buffer_type = MYSQL_TYPE_STRING;
    bind[i].buffer = (char *)param;
    bind[i].buffer_length = STRING_SIZE;
    bind[i].is_null = 0;
    bind[i].length = length;
}

void fillInputBindLong(MYSQL_BIND *bind, unsigned int i, int *param)
{

    bind[i].buffer_type = MYSQL_TYPE_LONG;
    bind[i].buffer = (char *)param;
    bind[i].is_null = 0;
    bind[i].length = 0;
}

void updateUserPassword(int userId, char *hash, char * salt)
{
    kore_log(1, "updatePassword");
    MYSQL *conn;
    MYSQL_STMT *statement;
    MYSQL_BIND input_bind[3];
    char hash_param[STRING_SIZE + 1];
    hash_param[STRING_SIZE] = NULL;
    char salt_param[STRING_SIZE + 1];
    salt_param[STRING_SIZE] = NULL;
    unsigned int i = 0;
    unsigned long str_length = STRING_SIZE;


    conn = mysql_init(NULL);

    _dbConnect(conn);
    char *query = "call update_password_for_userId(?,?,?)";

    statement = mysql_stmt_init(conn);
    if (!statement)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(statement, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(input_bind, 0, sizeof(input_bind));

    strncpy(hash_param, hash, STRING_SIZE); /* string  */
    strncpy(salt_param, salt, STRING_SIZE); /* string  */

    i = 0;
    fillInputBindLong(input_bind, i++, &userId);
    fillInputBindString(input_bind, i++, &hash_param, &str_length);
    fillInputBindString(input_bind, i++, &salt_param, &str_length);

    if (mysql_stmt_bind_param(statement, input_bind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(statement))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(statement))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(statement));
    }
}

DatabaseResult getAllBookedFlights(int userId)
{
    kore_log(1, " getAllBookedFlights");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND input_bind[1];
    unsigned int sizeOfOutPutBind = 7;

    unsigned int i = 0;

    MYSQL_BIND outputBind[sizeOfOutPutBind];

    /*values */
    int flightId_parm = 0,
        price_param = 0,
        capacity_param = 0,
        external_id_parm = 0;
    MYSQL_TIME date;

    char flight_source[STRING_SIZE + 1];
    flight_source[STRING_SIZE] = NULL;

    char flight_destination[STRING_SIZE + 1];
    flight_destination[STRING_SIZE] = NULL;

    // MySQL bool to make pointers fully compatible
    my_bool is_null[sizeOfOutPutBind];
    my_bool error[sizeOfOutPutBind];
    unsigned long length[sizeOfOutPutBind];

    conn = mysql_init(NULL);
    _dbConnect(conn);

    char *query = "call get_booked_flights_by_userId(?)";
    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    kore_log(2, query);
    kore_log(2, "%d", userId);

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }

    memset(input_bind, 0, sizeof(input_bind));

    fillInputBindLong(input_bind, 0, &userId);

    if (mysql_stmt_bind_param(stmt, input_bind))
    {
        kore_log(2, "ERROR");
    }

    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt))
    {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));

    i = 0;
    fillOutputBindLong(outputBind, i++, &flightId_parm, is_null, length, error);
    fillOutputBindDate(outputBind, i++, &date, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &price_param, is_null, length, error);
    fillOutputBindString(outputBind, i++, &flight_source, is_null, length, error);
    fillOutputBindString(outputBind, i++, &flight_destination, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &capacity_param, is_null, length, error);
    fillOutputBindLong(outputBind, i++, &external_id_parm, is_null, length, error);

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, outputBind))
    {
        kore_log(2, " mysql_stmt_bind_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }
    if (mysql_stmt_store_result(stmt))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult((unsigned int)mysql_stmt_num_rows(stmt), i);
    unsigned int y = 0;
    while (!mysql_stmt_fetch(stmt))
    {
        set_DatabaseResult(dbResult, y, db_flight_id, (char *)flightId_parm);

        SmartString *strDate = smart_string_new();

        char formattedDate[20];
        sprintf(formattedDate, "%02d-%02d-%04d", date.day, date.month, date.year);

        smart_string_append(strDate, formattedDate);
        set_DatabaseResult(dbResult, y, db_flight_date, strDate->buffer);

        set_DatabaseResult(dbResult, y, db_flight_price, (char *)price_param);

        SmartString *strFlight_source = smart_string_new();
        smart_string_append(strFlight_source, flight_source);
        set_DatabaseResult(dbResult, y, db_flight_flight_source, strFlight_source->buffer);

        SmartString *strFlight_destination = smart_string_new();
        smart_string_append(strFlight_destination, flight_destination);
        set_DatabaseResult(dbResult, y, db_flight_flight_destination, strFlight_destination->buffer);

        set_DatabaseResult(dbResult, y, db_flight_capacity, (char *)capacity_param);

        set_DatabaseResult(dbResult, y, db_flight_external_id, (char *)external_id_parm);
        y++;
    }

    return dbResult;
}