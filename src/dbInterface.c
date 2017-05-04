

#ifndef MYSQL
#include <mysql/mysql.h>
#endif


#define get_user_id_salt_hash_with_email 'select id,pasword_hash, pasword_salt from user where email = ?;';

void dbConnect(MYSQL *conn);
void getUsers();
void dbDisconnect(MYSQL *conn);

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

void getUsers()
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL *conn;
    int _row = 0,
        _column = 0;

    conn = mysql_init(NULL);
    dbConnect(conn);

    mysql_query(conn, "select * from user;");

    MYSQL_RES *result = mysql_store_result(conn);
    if (result != NULL)
    {
        unsigned int num_fields = mysql_num_fields(result);
        kore_log(2, "%d", result->row_count);
        kore_log(2, "%d", num_fields);
        char *array[result->row_count + 1][num_fields + 1];

        while ((row = mysql_fetch_row(result)))
        {
            for (_column = 0; _column < num_fields; _column++)
            {
                // kore_log(2, row[_column]);
                array[_row][_column] = row[_column];
            }
            _row++;
            kore_log(2, "");
        }
        // array[0][0] = 't';
        kore_log(2, " size of array: %d", (int) sizeof(array));

        for (_row = 0; _row < result->row_count; _row++)
        {
            for (_column = 0; _column < num_fields; _column++)
            {
                // kore_log(2, array[_row][_column]);
            }
        }
        struct kore_buf * buffer;
        buffer = kore_buf_alloc(sizeof(array));
        kore_buf_append(buffer, *array, sizeof(array));
        // Struct DatabaseResult rs = NULL;
        // rs->data = array;
        

    }
    dbDisconnect(conn);
}
