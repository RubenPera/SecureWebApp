#ifndef _kore_login_h_

#include <stdint.h>
#include <stdbool.h>

#define _kore_login_h_

#define LOGIN_HASH_ITERATIONS 4096

void login_hash_password(char *pass, char *salt, int32_t iterations, uint32_t outputBytes,
                         char *hexResult);

void generate_random(char *buffer, int buffer_length);

void *login_generate_salt(int length, char * output_buffer);

bool
login_validate_password(char *input_password, char *hash, char *salt); //compares input password to password in database


void login_process_password(char * password, char * hash_buff, char * salt_buff);
#endif
