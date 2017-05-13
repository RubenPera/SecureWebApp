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
    int rowCounter = 0,
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
        kore_log(2, " %c", 't');

        while ((row = mysql_fetch_row(result)))
        {
            for (columnCounter = 0; columnCounter < num_fields; columnCounter++)
            {
                set_DatabaseResult(dbResult, rowCounter, columnCounter, row[columnCounter]);
            }
            rowCounter++;
        }
    }

    _dbDisconnect(conn);
    return dbResult;
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