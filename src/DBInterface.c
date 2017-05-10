#include "Header.h"

#define get_user_id_salt_hash_with_email 'select id,pasword_hash, pasword_salt from user where email = ?;';

// Connects to database
// Should only be called from inside this file
void _dbConnect(MYSQL *conn)
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

void _dbDisconnect(MYSQL *conn)
{
    mysql_close(conn);
}

// Returns all users from database
// In a DatabaseResult struct
DatabaseResult getUsers()
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL *conn;
    int _row = 0,
        columnCounter = 0;
    DatabaseResult dbResult;

    conn = mysql_init(NULL);
    _dbConnect(conn);

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
    }

    _dbDisconnect(conn);
    return dbResult;
}

void getAllFlights()
{
    MYSQL_FIELD *field;
    MYSQL *conn;

    conn = mysql_init(NULL);
    _dbConnect(conn);

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

// Updates the session last_updated variable
// Session is deleted when older than 15 minutes
void update_session(int session_id)
{
    kore_log(2, "begin of function");
    MYSQL_FIELD *field;
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    int param_count;

    conn = mysql_init(NULL);
    kore_log(2, "conn inited");
    _dbConnect(conn);
    kore_log(2, "connected");
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

    kore_log(2, "param count = %s", param_count);

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
    kore_log(2, "2");
    // https://dev.mysql.com/doc/refman/5.6/en/mysql-stmt-execute.html
}