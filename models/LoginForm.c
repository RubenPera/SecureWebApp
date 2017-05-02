

typedef struct
{
	char * email;
	char * regex_email;
	char * password;
	char * regex_password;
} LoginFormModel;

void init_LoginFormModel(LoginFormModel *form);

void init_LoginFormModel(LoginFormModel *form) 
	//TODO: make valid regex for these parameters
	form->regex_email = "^\w*$";
	form->regex_password = "^\w*$";
}