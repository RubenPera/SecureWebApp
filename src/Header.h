#ifndef Header_H_
#define Header_H_

// Kore.io
#ifndef KORE_RESULT_OK
#include <kore/kore.h>
#include <kore/http.h>
#endif

#ifndef KORE_RESULT_OK
#include <kore.h>
#include <http.h>
#endif

#include "assets.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <openssl/sha.h>
#include <time.h>
#include "smart_string.h"
#include "../json-c/json.h"
#include <unistd.h>
#include <openssl/rand.h>
#include "database.h"
#define null NULL
#define STRING_SIZE 255

// DatabaseResult.c
typedef struct
{
    unsigned int rows;
    unsigned int columns;
    char ***data;
} DatabaseResult;

// SecureWebApp.c
int page(struct http_request *);
int login(struct http_request *);
int createUser(struct http_request *);
int bookFlight(struct http_request *);
int flightOverView(struct http_request *);


// Cookie.c
void setCookie(struct http_request *req, char *name, char *value, char *path);
struct kore_buf *getCookieValue(struct http_request *req, char *name);
void createSessionCookie(struct http_request *req, int user_id);

// DBInterface.c
void _dbConnect(MYSQL *conn);
DatabaseResult getUsers();
void _dbDisconnect(MYSQL *conn);
void update_session(int session_id);
DatabaseResult getSaltHashWithEmail(char email[STRING_SIZE]);

// DatabaseResult.c
DatabaseResult init_DatabaseResult(unsigned int rows, unsigned int columns);
char *get_DatabaseResult(DatabaseResult result, unsigned int row, unsigned int column);
void set_DatabaseResult(DatabaseResult result, unsigned int row, unsigned int column, char *value);

#endif