/*
login.c - SecureWebApp login library

This is a library to simplify user validation in the SecureWebApp project for InHolland Alkmaar

The library makes 3 functions accessable:

login_hash_password, which uses the PBKDF2 hashing algorithm to securely hash user passwords, and converts the output to hex
login_validate_password, which over a period of 1 second hashes the input password, compares the hashed password with the one provided in the input, and returns whether or not the 2 hashed password are equal
login_generate_salt, which generates salts for when a new password is created.

LOGIN_HASH_ITERATIONS is a constant defined in login.h, which can be used to ensure that every time the hash function is called, it uses the same amount of iterations
LOGIN_HASH_LENGTH     is a constant defined in login.h, which can be used to ensure that every time the hash function is called, the output hash is of the same length.

*/


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <mysql/mysql.h>
#include <stdbool.h>

#include <linux/random.h>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>
#include <sys/time.h>
#include <kore/kore.h>

#include "login.h"
#include "Header.h"
//#include "header.h"

#define LOGIN_DATABASE_ERROR 1
#define LOGIN_SUCCESS 0


//bool validate_password(LoginData * check, char * input_pass);
void hax_encode(char *input, int input_length, char *buffer);

void generate_random(char *buffer, int buffer_length);

uint64_t time_now();

void login_hash_password(char *pass, char *salt, int32_t iterations, uint32_t outputBytes,
                         char *hexResult) //hashes the password using PBKDF2 sha512
{
    unsigned char digest[outputBytes];
    PKCS5_PBKDF2_HMAC(pass, strlen(pass), salt, strlen(salt), iterations, EVP_sha512(), outputBytes, digest);
    hax_encode(digest, sizeof(digest), hexResult);
}

bool
login_validate_password(char *input_password, char *hash, char *salt) //compares input password to password in database
{
    char backup[STRING_SIZE + 1];
    strcpy(backup, hash);
    uint64_t start = time_now();        //get the starttime of the function in microseconds
    char hashed_input[LOGIN_HASH_LENGTH * 2 + 1]; //buffer for the hash function
    hashed_input[LOGIN_HASH_LENGTH * 2] = NULL;
    login_hash_password(input_password, salt, LOGIN_HASH_ITERATIONS, LOGIN_HASH_LENGTH,
                        hashed_input); //hash the password input by the user with the salt in the database

    int correct = 0;
    correct = strcmp(backup,
                     hashed_input); //check if the hashed password in the database and the hashed user input are equal
//    kore_log(2, "backup = %s", backup);
//    kore_log(2, "hashed = %s", hashed_input);
    while (time_now() < start + 1000);    //wait until the starttime + 1 second has passed

    return correct == 0;
}

uint64_t time_now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);    //get current time, write to timeval struct
    return tv.tv_sec * 1000 +
           tv.tv_usec / 1000; //change seconds and microseconds from timeval struct into single integer
}

char *login_generate_salt(int length) //generates the salt using /dev/urandom
{
    char buffer[length]; //create the buffer for the random data
    char *output_buffer = malloc(length * 2 + 1); //create buffer for b64 encoded data
    generate_random(buffer, length); //get the random data
    hax_encode(buffer, length, output_buffer);
    return output_buffer;
}

void generate_random(char *buffer, int buffer_length) //function for getting random data from /dev/urandom
{
    FILE *urandom = fopen("/dev/urandom", "r");
    fread(buffer, buffer_length, 1, urandom);
    fclose(urandom);
}


void hax_encode(char *input, int input_length,
                char *buffer) //this code is a modified version of the code found here: https://sourceforge.net/p/libb64/git/ci/master/tree/examples/c-example1.c
{ //encodes data into base64 using libb64-dev
    int i;
    for (i = 0; i < input_length; i++) {
        sprintf(buffer + (i * 2), "%02x", 255 & input[i]);
    }
}