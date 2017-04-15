# SecureWebApp

For this project the library kore.io is used. 
It is **strongly adviced** to follow the install tutorial on github of kore.io
https://github.com/jorisvink/kore

Because kore.io doesnt work well with projects pulled from git only the following folders are placed on github:
* src, for the c source files
* conf, for the configuration files
* view, for the html files

Because of this you should first create a project using kore.io and then import the github files into this project.

`kodev create SecureWebApp`<br />
`cd SecureWebApp/` <br />

You use checkout the files of this project into this folder.<br />
This is done with the following commands:<br />
`git init`<br />
`git remote add origin https://github.com/RubenPera/SecureWebApp`<br />
`git fetch`<br />
`git checkout -t origin/master`<br />

Then use `kodev run ` to start the application.


