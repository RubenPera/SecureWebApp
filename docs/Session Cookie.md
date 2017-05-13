# Secure loging in and cookies

## Session cookie
### In the database
Contains the following columns
|Column name|Discription|
|---|---|
|id|database id|
|user_id|Foreign key link to user|
|cookie_id|Random generated number that is the connection between the database and the cookie|
|last_used|DateTime, is used to keep track of the last time this session was used|



## User loging
When the user submits his login credentials they are validated with the database, when the credentials are incorrect the user stays in the same page. If the credentails are correct the following thing will happen:
- A Session cookie is created
- For this session a cookie_id is generated
- This cookie_id is combined with the user_id saved in the database in the Session table
- 