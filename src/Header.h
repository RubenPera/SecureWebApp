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

#define null NULL

// DatabaseResult.c
typedef struct{
    unsigned int rows;
    unsigned int columns;
    char*** data;
} DatabaseResult;

// SecureWebApp.c
int page(struct http_request *);
int login(struct http_request *);
int createUser(struct http_request *);

// Cookie.c
void setCookie(struct http_request * req, char * name, char * value, char * path);
struct kore_buf	* getCookieValue(struct http_request * req, char * name);

// DBInterface.c
void dbConnect(MYSQL *conn);
DatabaseResult getUsers();
void dbDisconnect(MYSQL *conn);

// DatabaseResult.c
DatabaseResult init_DatabaseResult(unsigned int rows, unsigned int columns);








#endif