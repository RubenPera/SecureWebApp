// #include <regex.h>

// typedef struct
// {
// 	char * email;
// 	char * regex_email;
// 	char * param_email;
// 	char * password;
// 	char * regex_password;
// 	char * param_password;
// 	char * birthdate;
// 	char * regex_birthdate;
// 	char * param_birthdate;
// 	char * address;
// 	char * regex_address;
// 	char * param_address;

// } CreateUserFormModel;

// CreateUserFormModel * new_CreateUserFormModel();
// CreateUserFormModel * fill_CreateUserFormModel(struct http_request *req);

// CreateUserFormModel * new_CreateUserFormModel() {
// 	CreateUserFormModel *form = malloc(sizeof(CreateUserFormModel));

// 	//TODO: make valid regex for these parameters
// 	form->regex_email = "^\w*$";
// 	form->param_email = "email";
// 	form->regex_password = "^\w*$";
// 	form->param_password = "password";
// 	form->regex_birthdate = "^(\d{1,2})-(\d{1,2})-(\d{4})$";
// 	form->param_birthdate = "birthdate";
// 	form->regex_address = "^\w*$";
// 	form->param_address = "address";

// 	return form;
// }

// CreateUserFormModel * fill_CreateUserFormModel(struct http_request *req) {
// 	CreateUserFormModel * model = new_CreateUserFormModel();
	
// 	http_populate_post(req);
// 	char * value_to_be_validated = "";
// 	http_argument_get_string(req, model->param_email, value_to_be_validated);

// 	return model;
	
	
// }