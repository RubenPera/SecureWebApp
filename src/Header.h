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
//#include "../json-c/json.h"
#include <json-c/json.h>
#include <unistd.h>
#include <openssl/rand.h>
#include "database.h"
#include "login.h"
#define null NULL
#define STRING_SIZE 256

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
int showLoginPage(struct http_request *req);


// Cookie.c
void setCookie(struct http_request *req, char *name, char *value);
void setCookieFull(struct http_request *req, char *name, char *value, char * expiryDate);
struct kore_buf *getCookieValue(struct http_request *req, char *name);
void createSessionCookie(struct http_request *req, int user_id);
void generateSessionKey(char key[STRING_SIZE]);
void removeCookie(struct http_request * req, char * name);
void removeSessionCookie(struct http_request * req);
void createSessionRow(int userId, char * sessionId);
int getUserIdWithSession(char sessionId[STRING_SIZE - 1]);
struct kore_buf *getSessionCookieValue(struct http_request *req);

// DBInterface.c
void _dbConnect(MYSQL *conn);
DatabaseResult getUsers();
void _dbDisconnect(MYSQL *conn);
void update_session(char * session_id);
DatabaseResult getIdSaltHashWithEmail(char *email);
void sqlToJson(SmartString *str, char *query, char *groupname);
void DoHet(SmartString *str);
void createBooking(int userId, char *flightId);
void getUserAirmiles(SmartString *output, int userId);


// DatabaseResult.c
DatabaseResult init_DatabaseResult(unsigned int rows, unsigned int columns);
char *get_DatabaseResult(DatabaseResult result, unsigned int row, unsigned int column);
void set_DatabaseResult(DatabaseResult result, unsigned int row, unsigned int column, char *value);

#endif