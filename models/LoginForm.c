

typedef struct
{
	char * email;
	char * regex_email;
	char * password;
	char * regex_password;

} LoginFormModel;

LoginFormModel * new_LoginForm();

LoginFormModel * new_LoginForm() {
	LoginFormModel *form = malloc(sizeof(LoginFormModel));


	//TODO: make valid regex for these parameters
	form->regex_email = "^\w*$";
	form->regex_password = "^\w*$";

	return form;
}