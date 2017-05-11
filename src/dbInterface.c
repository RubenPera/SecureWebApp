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