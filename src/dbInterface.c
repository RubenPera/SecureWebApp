#include "Header.h"

#define get_user_id_salt_hash_with_email 'select id,pasword_hash, pasword_salt from user where email = ?;';
void dbConnect(MYSQL *conn)
{
    char *server = "127.0.0.1";
    char *user = "root";
    char *password = "root"; /* set me first */
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

void dbDisconnect(MYSQL *conn)
{
    mysql_close(conn);
}

DatabaseResult getUsers()
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL *conn;
    int _row = 0,
        columnCounter = 0;
    DatabaseResult dbResult;

    conn = mysql_init(NULL);
    dbConnect(conn);

    mysql_query(conn, "call get_all_users()");

    MYSQL_RES *result = mysql_store_result(conn);
    if (result != NULL)
    {
        unsigned int num_fields = mysql_num_fields(result);

        dbResult = init_DatabaseResult(result->row_count, num_fields);

        while ((row = mysql_fetch_row(result)))
        {
            for (columnCounter = 0; columnCounter < num_fields; columnCounter++)
            {
                dbResult.data[_row][columnCounter] = row[columnCounter];
            }
            _row++;
        }
        for (int i = 0; i < dbResult.rows; i++)
        {
            for (int y = 0; y < dbResult.columns; y++)
            {
                kore_log(2, dbResult.data[i][y]);
            }
        }
    }

    dbDisconnect(conn);
    return dbResult;
}

// Updates the session last_updated variable
// Session is deleted when older than 15 minutes
void update_session(int session_id)
{
    kore_log(1, "update_session");
    MYSQL_FIELD *field;
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    int param_count;

    conn = mysql_init(NULL);
    _dbConnect(conn);
    char *query = "call update_session_last_use(?)";

    stmt = mysql_stmt_init(conn);
    kore_log(2, "stmt inited");
    if (!stmt)
    {
        kore_log(1, "mysql_stmt_init out of memory");
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        kore_log(1, "error");
    }
    param_count = mysql_stmt_param_count(stmt);

    kore_log(2, "param count = %d", param_count);

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&session_id;
    bind[0].is_null = 0;
    bind[0].length = 0;

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

DatabaseResult getSaltHashWithEmail(char email[STRING_SIZE])
{
    kore_log(1, " getSaltHashWithEmail");
    MYSQL_FIELD *field;
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND inputBind[1];
    MYSQL_BIND outputBind[2];
    MYSQL_RES *prepare_meta_result;

    unsigned long str_length = 255;
    int param_count, column_count, row_count;

    char email_param[STRING_SIZE];
    char salt_param[STRING_SIZE];

    char hash_param[STRING_SIZE];

    unsigned long length[2];
    bool is_null[2];
    bool error[2];

    conn = mysql_init(NULL);
    prepare_meta_result = mysql_stmt_result_metadata(stmt);
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

    inputBind[0].buffer_type = MYSQL_TYPE_STRING;
    inputBind[0].buffer = (char *)email_param;
    inputBind[0].buffer_length = STRING_SIZE;
    inputBind[0].is_null = 0;
    inputBind[0].length = &str_length;

    if (mysql_stmt_bind_param(stmt, inputBind))
    {
        kore_log(2, "ERROR");
    }

    strncpy(email_param, email, STRING_SIZE); /* string  */
    str_length = strlen(email_param);

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

    outputBind[0].buffer_type = MYSQL_TYPE_STRING;
    outputBind[0].buffer = (char *)salt_param;
    outputBind[0].buffer_length = STRING_SIZE;
    outputBind[0].is_null = &is_null[0];
    outputBind[0].length = &length[0];
    outputBind[0].error = &error[0];

    outputBind[1].buffer_type = MYSQL_TYPE_STRING;
    outputBind[1].buffer = (char *)hash_param;
    outputBind[1].buffer_length = STRING_SIZE;
    outputBind[1].is_null = &is_null[1];
    outputBind[1].length = &length[1];
    outputBind[1].error = &error[1];

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
    row_count = 0;
    mysql_stmt_fetch(stmt);
    mysql_stmt_close(stmt);
    _dbDisconnect(conn);
    DatabaseResult dbResult;
    dbResult = init_DatabaseResult(1, 2);
    set_DatabaseResult(dbResult, 0, 0, salt_param);
    set_DatabaseResult(dbResult, 0, 1, hash_param);
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
    dbConnect(conn);

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

    dbDisconnect(conn);
    smart_string_append(str, json_object_to_json_string(container));
}

void DoHet(SmartString *str)
{
    smart_string_append(str, "hallo");
}

void createBooking(char *userId, char *flightId)
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
    dbConnect(conn);

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

    if (userAirmiles > flightPrice)
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

    /*Disconnect from db*/
    dbDisconnect(conn);

    /*Destroy/ Clean up smartstrings*/
    smart_string_destroy(updateFlight);
    smart_string_destroy(updateBooking);
    smart_string_destroy(getFlightPriceQuery);
    smart_string_destroy(getUserAirmilesQuery);
    smart_string_destroy(updateUserAirmilesQuery);
}

void getUserAirmiles(SmartString *output, char *userId)
{
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    SmartString *getUserAirmilesQuery = smart_string_new();

    /*connect to db*/
    conn = mysql_init(NULL);
    dbConnect(conn);

    createGetUserAirmilesQuery(getUserAirmilesQuery, userId);

    mysql_query(conn, getUserAirmilesQuery->buffer);

    result = mysql_store_result(conn);
    if (result != NULL)
    {
        row = mysql_fetch_row(result);
        smart_string_append(output, row[0]);
    }
    dbDisconnect(conn);
}
void createInsertBookingQuery(SmartString *str, char *userId, char *flightId)
{
    smart_string_append(str, "call insert_booking(");
    smart_string_append(str, userId);
    smart_string_append(str, ",");
    smart_string_append(str, flightId);
    smart_string_append(str, ");");
}

void createUpdateFlightCapacityQuery(SmartString *str, char *flightId)
{
    smart_string_append(str, "call update_flight_capacity(");
    smart_string_append(str, flightId);
    smart_string_append(str, ");");
}

void createGetFlightPriceQuery(SmartString *str, char *flightId)
{
    smart_string_append(str, "call get_flight_price(");
    smart_string_append(str, flightId);
    smart_string_append(str, ");");
}

void createGetUserAirmilesQuery(SmartString *str, char *userId)
{
    smart_string_append(str, "call get_user_airmiles_by_userid(");
    smart_string_append(str, userId);
    smart_string_append(str, ");");
}

void createUpdateUserAirMilesQuery(SmartString *str, char *userId, int price)
{
    smart_string_append(str, "call update_user_airmiles(");
    smart_string_append(str, userId);
    smart_string_append(str, ",");
    smart_string_append_sprintf(str, "%d", price);
    smart_string_append(str, ");");
}
