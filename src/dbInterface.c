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

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)sessionId_param;
    bind[0].buffer_length = STRING_SIZE;
    bind[0].is_null = 0;
    bind[0].length = &str_length;

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
    inputBind[i].buffer_type = MYSQL_TYPE_STRING;
    inputBind[i].buffer = (char *)email_param;
    inputBind[i].buffer_length = STRING_SIZE;
    inputBind[i].is_null = 0;
    inputBind[i].length = &str_length;

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
    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
    outputBind[i].buffer = (char *)&userId;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
    outputBind[i].buffer = (char *)salt_param;
    outputBind[i].buffer_length = STRING_SIZE;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
    outputBind[i].buffer = (char *)hash_param;
    outputBind[i].buffer_length = STRING_SIZE;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

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
    dbResult = init_DatabaseResult(1, i + 1);
    i = 0;
    set_DatabaseResult(dbResult, 0, i++, (char *)userId);
    set_DatabaseResult(dbResult, 0, i++, salt_param);
    set_DatabaseResult(dbResult, 0, i++, hash_param);

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
    if (result != NULL)
    {
        kore_log(2, getUserAirmilesQuery->buffer);
        userRow = mysql_fetch_row(result);
        userAirmiles = atoi(userRow[0]);
        kore_log(2, userRow[0]);
        mysql_free_result(result);
        result = NULL;
        mysql_next_result(conn);
    }
    else
    {
        kore_log(2, "Error: ResultNullexception");
    }

    mysql_query(conn, getFlightPriceQuery->buffer);
    result = mysql_store_result(conn);

    if (result != NULL)
    {
        kore_log(2, getFlightPriceQuery->buffer);
        flightRow = mysql_fetch_row(result);
        flightPrice = atoi(flightRow[0]);
        kore_log(2, flightRow[0]);
        result = NULL;
        mysql_next_result(conn);
    }
    else
    {
        kore_log(2, "Error: ResultNullexception");
    }

    if (userAirmiles >= flightPrice)
    {
        /*Create a query for decreasing capacity of flight by 1 */
        createUpdateFlightCapacityQuery(updateFlight, flightId);
        kore_log(2, updateFlight->buffer);
        if (mysql_query(conn, updateFlight->buffer) != 0)
        {
            kore_log(2, "UPDATE Flight failed: Stopping booking..");
        }
        else
        {

            /*Create a qeury for inserting a booking into the booking table */
            createInsertBookingQuery(updateBooking, userId, flightId);
            kore_log(2, updateBooking->buffer);
            if (mysql_query(conn, updateBooking->buffer) != 0)
            {
                kore_log(2, "INSERT INTO Booking failed: Stopping booking..");
            }
            else
            {

                createUpdateUserAirMilesQuery(updateUserAirmilesQuery, userId, flightPrice);
                kore_log(2, updateUserAirmilesQuery->buffer);
                if (mysql_query(conn, updateUserAirmilesQuery->buffer) != 0)
                {
                    kore_log(2, "UPDATE User failed: Stopping booking..");
                }
                else
                {
                    kore_log(2, "Booking transaction succeeded.");
                }
            }
        }
    }
    else
    {
        kore_log(2, "not enought moneys");
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

void getUserAirmiles(SmartString *output, int userId)
{
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
    if (result != NULL)
    {
        row = mysql_fetch_row(result);
        smart_string_append(output, row[0]);
    }
    _dbDisconnect(conn);
}

void createInsertBookingQuery(SmartString *str, int userId, char *flightId)
{
    smart_string_append(str, "call insert_booking(");
    smart_string_append_sprintf(str, "%d", userId);
    smart_string_append(str, ",");
    smart_string_append_sprintf(str, "%d", flightId);
    smart_string_append(str, ");");
}

void createUpdateFlightCapacityQuery(SmartString *str, int flightId)
{
    smart_string_append(str, "call update_flight_capacity(");
    smart_string_append_sprintf(str, "%d", flightId);
    smart_string_append(str, ");");
}

void createGetFlightPriceQuery(SmartString *str, int flightId)
{
    smart_string_append(str, "call get_flight_price(");
    kore_log(2, "flightId = %d", flightId);
    smart_string_append_sprintf(str, "%d", flightId);
    smart_string_append(str, ");");
}

void createGetUserAirmilesQuery(SmartString *str, int userId)
{
    smart_string_append(str, "call get_user_airmiles_by_userid(");
    smart_string_append_sprintf(str, "%d", userId);
    smart_string_append(str, ");");
}

void createUpdateUserAirMilesQuery(SmartString *str, int userId, int price)
{
    smart_string_append(str, "call update_user_airmiles(");
    smart_string_append_sprintf(str, "%d", userId);
    smart_string_append(str, ",");
    smart_string_append_sprintf(str, "%d", price);
    smart_string_append(str, ");");
}

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

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&userId;
    bind[0].is_null = 0;
    bind[0].length = 0;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char *)session_id_param;
    bind[1].buffer_length = STRING_SIZE;
    bind[1].is_null = 0;
    bind[1].length = &str_length;

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

    inputBind[0].buffer_type = MYSQL_TYPE_STRING;
    inputBind[0].buffer = (char *)session_id_param;
    inputBind[0].buffer_length = STRING_SIZE;
    inputBind[0].is_null = 0;
    inputBind[0].length = &str_length;

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

    outputBind[0].buffer_type = MYSQL_TYPE_LONG;
    outputBind[0].buffer = (char *)&userId;
    outputBind[0].is_null = &is_null[0];
    outputBind[0].length = &length[0];
    outputBind[0].error = &error[0];

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

    inputBind[0].buffer_type = MYSQL_TYPE_LONG;
    inputBind[0].buffer = (char *)&userId;
    inputBind[0].is_null = 0;
    inputBind[0].length = 0;

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
    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
    outputBind[i].buffer = (char *)&id_param;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
    outputBind[i].buffer = (char *)salt_param;
    outputBind[i].buffer_length = STRING_SIZE;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
    outputBind[i].buffer = (char *)hash_param;
    outputBind[i].buffer_length = STRING_SIZE;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
    outputBind[i].buffer = (char *)email_param;
    outputBind[i].buffer_length = STRING_SIZE;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
    outputBind[i].buffer = (char *)&miles_param;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
    outputBind[i].buffer = (char *)&role_param;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

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
    dbResult = init_DatabaseResult(1, i + 1);

    i = 0;
    set_DatabaseResult(dbResult, 0, i++, (char *)id_param);
    set_DatabaseResult(dbResult, 0, i++, hash_param);
    set_DatabaseResult(dbResult, 0, i++, salt_param);
    set_DatabaseResult(dbResult, 0, i++, email_param);
    set_DatabaseResult(dbResult, 0, i++, (char *) miles_param);
    set_DatabaseResult(dbResult, 0, i++, (char *) role_param);
    kore_log(2, "param miles = %d", miles_param);
    set_DatabaseResult(dbResult, 0, i++, (char *)miles_param);
    set_DatabaseResult(dbResult, 0, i++, (char *)role_param);

    return dbResult;
}

DatabaseResult getFlightById(int flightId)
{
    MYSQL *conn;
    MYSQL_STMT *statement;
    conn = mysql_init(NULL);
    DatabaseResult dbResult;
    

    _dbConnect(conn);
    statement = mysql_stmt_init(conn);

    if (statement == NULL)
    {
        kore_log(2, "ERROR:mysql_stmt_init() failed.\n");
        return dbResult;        
    }

    char *query = "call get_flight_by_id(?)";

    if (mysql_stmt_prepare(statement, query, strlen(query)))
    {
        kore_log(2, "ERROR:mysql_stmt_prepare() failed. Error:%s\nsql:%s\n", mysql_stmt_error(statement), query);
        return dbResult;
    
    }

    MYSQL_BIND input_bind[1];
    memset(input_bind, 0, sizeof(input_bind));
    int fid = flightId;
    unsigned long fid_len = sizeof(fid);

    /*Bind input*/
    input_bind[0].buffer_type = MYSQL_TYPE_LONG;
    input_bind[0].buffer = &fid;
    input_bind[0].buffer_length = sizeof(fid);
    input_bind[0].length = &fid_len;
    input_bind[0].is_null = (my_bool *)0;

    if (mysql_stmt_bind_param(statement, input_bind))
    {
        kore_log(2, "ERROR:mysql_stmt_bind_param failed\n");
        return dbResult;
        
    }

    if (mysql_stmt_execute(statement))
    {
        kore_log(2, "mysql_stmt_execute(), failed. Error:%s\n", mysql_stmt_error(statement));
        return dbResult;
    
    }

    /* Fetch result set meta information */
    MYSQL_RES *prepare_meta_result = mysql_stmt_result_metadata(statement);
    if (!prepare_meta_result)
    {
        kore_log(2, " mysql_stmt_result_metadata(), returned no meta information\n");
        kore_log(2, " %s\n", mysql_stmt_error(statement));
        return dbResult;
        
    }

    /* Get total columns in the query */
    int column_count = mysql_num_fields(prepare_meta_result);
    if (column_count != 6) /* validate column count */
    {
        kore_log(2, " invalid column count returned by MySQL\n");
        return dbResult;        
    }
    /* Bind result columns expected*/
    MYSQL_BIND result_bind[6];
    memset(result_bind, 0, sizeof(result_bind));

    my_bool result_is_null[6];
    my_bool error[6];
    unsigned long length[6];
    int flight_id = 0;
    char flight_date[STRING_SIZE];
    int flight_price = 0;
    char flight_source[STRING_SIZE];
    char flight_destination[STRING_SIZE];
    int capacity = 0;

    unsigned int i = 0;
    /* id */
    result_bind[i].buffer_type = MYSQL_TYPE_LONG;
    result_bind[i].buffer = (char *)&flight_id;
    result_bind[i].is_null = &result_is_null[i];
    result_bind[i].buffer_length = sizeof(flight_id);
    result_bind[i].length = &length[i];
    result_bind[i].error = &error[i];

    /* date */
    i++;
    result_bind[i].buffer_type = MYSQL_TYPE_STRING;
    result_bind[i].buffer = flight_date;
    result_bind[i].is_null = &result_is_null[i];
    result_bind[i].buffer_length = sizeof(flight_date);
    result_bind[i].length = &length[i];
    result_bind[i].error = &error[i];

    /* price */
    i++;
    result_bind[i].buffer_type = MYSQL_TYPE_LONG;
    result_bind[i].buffer = (char *)&flight_price;
    result_bind[i].is_null = &result_is_null[i];
    result_bind[i].buffer_length = sizeof(flight_price);
    result_bind[i].length = &length[i];
    result_bind[i].error = &error[i];

    /* flight_source */
    i++;
    result_bind[i].buffer_type = MYSQL_TYPE_STRING;
    result_bind[i].buffer = (char *)flight_source;
    result_bind[i].is_null = &result_is_null[i];
    result_bind[i].buffer_length = sizeof(flight_source);
    result_bind[i].length = &length[i];
    result_bind[i].error = &error[i];

    /* flight_destination */
    i++;
    result_bind[i].buffer_type = MYSQL_TYPE_STRING;
    result_bind[i].buffer = (char *)flight_destination;
    result_bind[i].is_null = &result_is_null[i];
    result_bind[i].buffer_length = sizeof(flight_destination);
    result_bind[i].length = &length[i];
    result_bind[i].error = &error[i];

    /* capacity */
    i++;
    result_bind[i].buffer_type = MYSQL_TYPE_LONG;
    result_bind[i].buffer = (char *)&capacity;
    result_bind[i].is_null = &result_is_null[i];
    result_bind[i].buffer_length = sizeof(capacity);
    result_bind[i].length = &length[i];
    result_bind[i].error = &error[i];

    if (mysql_stmt_bind_result(statement, result_bind))
    {
        kore_log(2, "mysql_stmt_bind_Result(), failed. Error:%s\n", mysql_stmt_error(statement));
        return dbResult;
        
    }

    if (mysql_stmt_store_result(statement))
    {
        kore_log(2, " mysql_stmt_store_result() failed\n");
        kore_log(2, " %s\n", mysql_stmt_error(statement));
        return dbResult;
    }

    mysql_stmt_fetch(statement);
    _dbDisconnect(conn);

    dbResult = init_DatabaseResult(1, i + 1);

    i = 0;
    set_DatabaseResult(dbResult, 0, i++, (char *)flight_id);
    set_DatabaseResult(dbResult, 0, i++, flight_date);
    set_DatabaseResult(dbResult, 0, i++, (char *)flight_price);
    set_DatabaseResult(dbResult, 0, i++, flight_source);
    set_DatabaseResult(dbResult, 0, i++, flight_destination);
    set_DatabaseResult(dbResult, 0, i++, (char *)capacity);

kore_log(2,"%d",capacity);

    return dbResult;
}

//DatabaseResult getAllFlights(){
//    kore_log(1, "getAllFlights");
//    MYSQL *conn;
//    MYSQL_STMT *stmt;
//    MYSQL_BIND outputBind[6];
//
//    unsigned long length[6];
//    // MySQL bool to make pointers fully compatible
//    my_bool is_null[6];
//    my_bool error[6];
//
//    unsigned long str_length = STRING_SIZE;
//
//    char email_param[STRING_SIZE];
//    char salt_param[STRING_SIZE + 1];
//    salt_param[STRING_SIZE] = NULL;
//
//    char hash_param[STRING_SIZE + 1];
//    hash_param[STRING_SIZE] = NULL;
//
//    int id_param = 0,
//            miles_param = 0,
//            role_param = 0;
//
//
//    conn = mysql_init(NULL);
//    _dbConnect(conn);
//    char *query = "call get_all_users()";
//
//    stmt = mysql_stmt_init(conn);
//    if (!stmt) {
//        kore_log(1, "mysql_stmt_init out of memory");
//    }
//
//    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
//        kore_log(1, "error");
//    }
//
//    if (mysql_stmt_store_result(stmt)) {
//        kore_log(2, " mysql_stmt_store_result() failed\n");
//    }
//
//    if (mysql_stmt_execute(stmt)) {
//        kore_log(2, "ERROR executing");
//        kore_log(2, " %s\n", mysql_stmt_error(stmt));
//    }
//
//    memset(outputBind, 0, sizeof(outputBind));
//
//    unsigned int i = 0;
//    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
//    outputBind[i].buffer = (char *) &id_param;
//    outputBind[i].is_null = &is_null[i];
//    outputBind[i].length = &length[i];
//    outputBind[i].error = &error[i];
//
//    i++;
//    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
//    outputBind[i].buffer = (char *) salt_param;
//    outputBind[i].buffer_length = STRING_SIZE;
//    outputBind[i].is_null = &is_null[i];
//    outputBind[i].length = &length[i];
//    outputBind[i].error = &error[i];
//
//    i++;
//    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
//    outputBind[i].buffer = (char *) hash_param;
//    outputBind[i].buffer_length = STRING_SIZE;
//    outputBind[i].is_null = &is_null[i];
//    outputBind[i].length = &length[i];
//    outputBind[i].error = &error[i];
//
//    i++;
//    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
//    outputBind[i].buffer = (char *) email_param;
//    outputBind[i].buffer_length = STRING_SIZE;
//    outputBind[i].is_null = &is_null[i];
//    outputBind[i].length = &length[i];
//    outputBind[i].error = &error[i];
//
//    i++;
//    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
//    outputBind[i].buffer = (char *) &miles_param;
//    outputBind[i].is_null = &is_null[i];
//    outputBind[i].length = &length[i];
//    outputBind[i].error = &error[i];
//
//    i++;
//    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
//    outputBind[i].buffer = (char *) &role_param;
//    outputBind[i].is_null = &is_null[i];
//    outputBind[i].length = &length[i];
//    outputBind[i].error = &error[i];
//
//    /* Bind the result buffers */
//    if (mysql_stmt_bind_result(stmt, outputBind)) {
//        kore_log(2, " mysql_stmt_bind_result() failed\n");
//        kore_log(2, " %s\n", mysql_stmt_error(stmt));
//    }
//    if (mysql_stmt_store_result(stmt)) {
//        kore_log(2, " mysql_stmt_store_result() failed\n");
//        kore_log(2, " %s\n", mysql_stmt_error(stmt));
//    }
//
//    _dbDisconnect(conn);
//    DatabaseResult dbResult;
//    dbResult = init_DatabaseResult(mysql_stmt_num_rows(stmt), i + 1);
//    unsigned int y = 0;
//    while (!mysql_stmt_fetch(stmt)) {
//
//        i = 0;
//        set_DatabaseResult(dbResult, y, i++, (char *) id_param);
//        set_DatabaseResult(dbResult, y, i++, hash_param);
//        set_DatabaseResult(dbResult, y, i++, salt_param);
//        set_DatabaseResult(dbResult, y, i++, email_param);
//        set_DatabaseResult(dbResult, y, i++, (char *) miles_param);
//        set_DatabaseResult(dbResult, y, i++, (char *) role_param);
//        y++;
//    }
//
//    return dbResult;
//}

DatabaseResult getAllUsers() {
    kore_log(1, "getAllUsers");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND outputBind[6];

    unsigned long length[6];
    // MySQL bool to make pointers fully compatible
    my_bool is_null[6];
    my_bool error[6];

    unsigned long str_length = STRING_SIZE;

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
    if (!stmt) {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        kore_log(1, "error");
    }

    if (mysql_stmt_store_result(stmt)) {
        kore_log(2, " mysql_stmt_store_result() failed\n");
    }

    if (mysql_stmt_execute(stmt)) {
        kore_log(2, "ERROR executing");
        kore_log(2, " %s\n", mysql_stmt_error(stmt));
    }

    memset(outputBind, 0, sizeof(outputBind));

    unsigned int i = 0;
    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
    outputBind[i].buffer = (char *) &id_param;
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

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_STRING;
    outputBind[i].buffer = (char *) email_param;
    outputBind[i].buffer_length = STRING_SIZE;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
    outputBind[i].buffer = (char *) &miles_param;
    outputBind[i].is_null = &is_null[i];
    outputBind[i].length = &length[i];
    outputBind[i].error = &error[i];

    i++;
    outputBind[i].buffer_type = MYSQL_TYPE_LONG;
    outputBind[i].buffer = (char *) &role_param;
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

    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult(mysql_stmt_num_rows(stmt), i + 1);
    unsigned int y = 0;
    while (!mysql_stmt_fetch(stmt)) {

        i = 0;
        set_DatabaseResult(dbResult, y, db_user_id, (char *) id_param);
        set_DatabaseResult(dbResult, y, db_user_pasword_hash, hash_param);
        set_DatabaseResult(dbResult, y, db_user_pasword_salt, salt_param);
        SmartString *str = smart_string_new();
        smart_string_append(str, email_param);

        set_DatabaseResult(dbResult, y, db_user_email, str->buffer);

        set_DatabaseResult(dbResult, y, db_user_inholland_miles, (char *) miles_param);
        set_DatabaseResult(dbResult, y, db_user_role, (char *) role_param);
        y++;
    }

    return dbResult;
}

void setUserNewAirMiles(int userId, int airMiles){

    kore_log(1, "setUserNewAirMiles");
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND inputBind[2];
    unsigned int i = 0;
    conn = mysql_init(NULL);
    _dbConnect(conn);
    char *query = "call set_airMiles_for_userId(?,?)";

    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        kore_log(1, "error");
    }

    memset(inputBind, 0, sizeof(inputBind));

    inputBind[i].buffer_type = MYSQL_TYPE_LONG;
    inputBind[i].buffer = (char *) &userId;
    inputBind[i].is_null = 0;
    inputBind[i].length = 0;

    i++;
    inputBind[i].buffer_type = MYSQL_TYPE_LONG;
    inputBind[i].buffer = (char *) &airMiles;
    inputBind[i].is_null = 0;
    inputBind[i].length = 0;


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
}
