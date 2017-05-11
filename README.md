# SecureWebApp

## Intro
For this project the library kore is used. 
It is **strongly adviced** to follow the install tutorial on github of kore
[Kore.io](https://github.com/jorisvink/kore)

# Install
Clone the kore repository to a desired directory:

`git clone https://github.com/jorisvink/kore`

Navigate to the /kore folder and install kore
`make` 
`make install`


# Build Project
Because kore doesnt work well with projects pulled from git only the following folders are placed on github:
* src, for the c source files
* conf, for the configuration files
* view, for the html files

Because of this you should first create a project using kore and then import the github files into this project.

`kodev create SecureWebApp`
`cd SecureWebApp/` 


You use checkout the files of this project into this folder.
This is done with the following commands:
`git init`  
`git remote add origin https://github.com/RubenPera/SecureWebApp`  
`git fetch`  
`rm .gitignore conf/SecureWebApp.conf src/SecureWebApp.c`  
`git checkout -t origin/master`  
`kodev build`


Because mysql is now used in the application the mysql libraries must be linked in the project.  
First make sure mysql and the developer environment is installed:  
`apt-get install mysql-client libmysqlclient-dev`


Now add the mysql link to conf/build.conf. Run the following command   
`mysql_config --cflags --libs`

And place this output in the build.conf file, it must be preceeded by `ldflags=` be on a newline and in thet shared configurations.
Example:  
`ldflags= -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -ldl`

Then use `kodev run ` to start the application.

# Dependencies 
For the project to build succesfuly, download and install these dependencies.

@JSON-C - JSON BUILDER  

-Any Linux: https://github.com/json-c/json-c<br>
-Ubuntu: https://linuxprograms.wordpress.com/2010/05/20/install-json-c-in-linux/<br>
-Packet Manager.<br>

After installing, add the following lines to build.conf:

`ldflags=-I/usr/include/json-c`<br>
`CFLAGS =-I/user/include/json-c`<br>
`LDFLAGS=-L/user/lib -ljson-c`<br>


## Known Errors

If the command kodev run gives an error like: 
`" on line xx`
This is due the line ending being a windows line ending instead of an Unix line ending. 

Use the command 
`dos2unix SecureWebApp.conf`

### Kore install fals
Make sure openssl dev is installed, installl this with:
`sudo apt-get install libssl-dev`
