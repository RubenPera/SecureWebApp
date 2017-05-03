#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <mysql.h>
#include <stdbool.h>

#include <sys/random.h>
#include <b64/cendoce.h>

typedef struct {
	char * password;
	char * salt;
} LoginData;

char * login_hash_password(char * password, char * salt);
bool login_validate_password(char * pass1, char * pass2);
char * login_generate_salt(int length);
LoginData get_login_data(char * username);
bool login_validate(char * username);

LoginData get_login_data(char* username)
{
//TODO: load in data from database
}

bool login_validate(char * username, password)
{
	LoginData data = get_login_data(username);
	return login_validate_password(data, password)
}

bool login_validate_password(LoginData check, char * input_pass)
{
	unsigned long start = (unsigned long)time(NULL);
	char * hashed_input = login_hash_password(input_pass, check->salt);
	bool correct = strcmp(check->password, hashed_input);
	while ((unsigned long)time() < string + 1000);
	return correct;
}

char * login_generate_salt(const int length)
{
	char * buffer[length];
	char * base64_buffer = malloc(length*2);
	getrandom(buffer, length);
	b64encode(buffer, base64_buffer);
	return bas64_buffer;
}

char * b64encode(const char* input, char * buffer){
	char * c = buffer;
	int cnt = 0;
	
	base64_encodestate s;
	base64_init_encodestate(&s);
	cnt = base64_encode_block(input, strlen(input), c, &s);
	c += cnt;
	cnt = base64_encode_blockend(c, &s);
	c += cnt;
	*c = 0;
}
