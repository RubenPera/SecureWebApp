# SecureWebApp

## Intro
For this project the library kore.io is used. 
It is **strongly adviced** to follow the install tutorial on github of kore.io
https://github.com/jorisvink/kore

# Install
Clone the kore repository to a desired directory:

`git clone https://github.com/jorisvink/kore`

Navigate to the /kore folder and install kore
`make` <br>
`make install`<br>


# Build Project
Because kore.io doesnt work well with projects pulled from git only the following folders are placed on github:
* src, for the c source files
* conf, for the configuration files
* view, for the html files

Because of this you should first create a project using kore.io and then import the github files into this project.

`kodev create SecureWebApp`<br />
`cd SecureWebApp/` 


You use checkout the files of this project into this folder.<br />
This is done with the following commands:<br />
`git init`<br />
`git remote add origin https://github.com/RubenPera/SecureWebApp`<br />
`git fetch`<br />
`rm .gitignore conf/SecureWebApp.conf src/SecureWebApp.c`<br />
`git checkout -t origin/master`<br />
`kodev build`


Because mysql is now used in the application the mysql libraries must be linked in the project.<br>
First make sure mysql and the developer environment is instlaled:<br>
`apt-get install mysql-client libmysqlclient-dev`


Now add the mysql link to conf/build.conf. Run the following command<br>
`mysql_config --cflags --libs`<br>
And place this output in the build.conf file, it must be preceeded by `ldflags=` be on a newline and in thet shared configurations.<br>
Example:<br>
`ldflags= -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -ldl`


Then use `kodev run ` to start the application.

## Known Errors

If the command kodev run gives an error like: <br />
`" on line xx`<br />
This is due the line ending being a windows line ending instead of an Unix line ending. 

Use the command <br />
`dos2unix SecureWebApp.conf`

### Kore install fals
Make sure openssl dev is installed, installl this with: <br>
`sudo apt-get install libssl-dev`

