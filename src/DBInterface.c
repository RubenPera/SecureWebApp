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

void getAllFlights()
{
    MYSQL_FIELD *field;
    MYSQL *conn;

    conn = mysql_init(NULL);
    dbConnect(conn);

    mysql_query(conn, "select * from flight;");

    MYSQL_RES *result = mysql_store_result(conn);
    int num_fields = mysql_num_fields(result);
    char **field_array;
    field_array = (char **)malloc(sizeof(char *) * (num_fields + 1));
    for (int i = 0; i < num_fields; i++)
    {
        field = mysql_fetch_field(result);
        field_array[i] = field->name;
    }

    for (int i = 0; i < num_fields; i++)
    {
        kore_log(2, field_array[i]);
    }
}