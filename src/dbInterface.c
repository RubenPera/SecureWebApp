

#ifndef MYSQL
#include <mysql/mysql.h>
#endif

#define get_user_id_salt_hash_with_email 'select id,pasword_hash, pasword_salt from user where email = ?;';

int dbConnect();

int dbConnect()
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "127.0.0.1";
    char *user = "root";
    char *password = "root"; /* set me first */
    char *database = "sys";

    kore_log(2, mysql_get_client_info());

    /* Connect to database */
    if (mysql_real_connect(conn, server,
                           user, password, database, 3306, NULL, 0))
    {
        kore_log(2, mysql_error(conn));
    }
    kore_log(2, "test");
    mysql_select_db(conn, "sys");
    kore_log(2, "db selected   ");
    mysql_query(conn, "select * from users;");

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL)
    {
        kore_log(2, "nothing found");
    }
    else
    {
        MYSQL_ROW row;

        kore_log(2, "result is not null");
        unsigned long *lengths = mysql_fetch_lengths(result);
        kore_log(2, "MySQL Tables in mysql database:");

        while ((row = mysql_fetch_row(result)))
        {
            kore_log(2, row[1]);
        }

        while ((row = mysql_fetch_row(res)) != NULL)
        {
            kore_log(2, row[0]);
        }
    }
}