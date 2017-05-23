create database secure_web_app_database;

use secure_web_app_database;

create table user(
	id int not null auto_increment,
    pasword_hash varchar(265) not null,
    pasword_salt varchar(265) not null,
    email varchar(265) not null,
    inholland_miles int not null,
    paspoort_id varchar(265) not null,
    paspoort_valid_date varchar(265) not null,
    role int not null,
	primary key (id) 
    );
    
create table flight(
	id int not null auto_increment,
    date datetime not null,
    price int not null,
    flight_source varchar(265) not null,
    flight_destination varchar(265) not null,
    capacity int not null,
   	primary key (id) 
    );
    
-- INSERT INTO flight
-- VALUES(NULL,'2017-06-3','%d-%m-%y'),'9991','Rotterdam','Notterdam','200');




create table booking(
	id int not null auto_increment,
    user_id int not null,
    flight_id int not null,
    primary key (id),
    foreign key (user_id) references user (id),
    foreign key (flight_id) references flight (id)
    );

create table session(
	id int not null auto_increment,
    user_id int not null,
    last_use datetime not null default now(),
    primary key (id),
    foreign key (user_id) references user (id)
    );