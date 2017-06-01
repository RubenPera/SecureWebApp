use secure_web_app_database;
alter table session
add session_id int not null;

SET SQL_SAFE_UPDATES = 0;
SET GLOBAL event_scheduler = ON;
-- create stored procedure
delimiter //
create procedure purge_session()
begin
	delete from session
	where date_sub(now(), interval 15 minute) > session.last_use;
end //
delimiter ;

create event e_purge_session
	on schedule
		every 1 minute
	do
		call purge_session();

delimiter //
create procedure get_user(in emailParam varchar(255))
	begin
		select *
        from user
        where user.email = emailParam;
	end //
delimiter ;

delimiter //
create procedure get_all_users()
	begin
		select *
        from user;
	end //

delimiter //
create procedure update_flight_capacity(in flightId varchar(255))
	begin
		UPDATE flight 
		SET capacity = capacity - 1 
		WHERE id = flightId;
	end //
delimiter ;

delimiter //
create procedure insert_booking(in userId varchar(255),in flightId varchar(255))
	begin
		INSERT INTO booking 
		VALUES (NULL,userId,flightId);
	end //
delimiter ;

delimiter //
create procedure get_all_flights()
	begin
		select *,DATE_FORMAT(date,'%d/%m/%Y') as 'date'
        from flight;
	end //

delimiter;

delimiter //
create procedure get_user_by_id(in userId varchar(255))
	begin
		select *
        from user
        where id = userId;
	end //
delimiter ;

delimiter //
create procedure get_user_airmiles_by_userid(in userId varchar(255))
	begin
		select inholland_miles
        from user
        where id = userId;
	end //
delimiter ;

delimiter //
create procedure get_flight_price(in flightId varchar(255))
	begin
		select price
        from flight
        where id = flightId;
	end //
delimiter ;

delimiter //
create procedure update_user_airmiles(in userId varchar(255),in price int)
	begin
		UPDATE user 
		SET inholland_miles = inholland_miles - price
        where id = userId;
	end //
delimiter ;

delimiter //
create procedure update_session_last_use(in session_id_var int(11))
	begin
		update session
		set last_use = now()
		where session_id = session_id_var;
	end //
delimiter ;

delimiter //
create procedure get_user_with_session(in session_id_var int(11))
	begin
		select * from user
		where id = (
		select session.user_id 
		from session
		where session.session_id = session_id_var);
	end //
delimiter ;

delimiter //
create procedure get_user_with_email(in email_var varchar(255))
	begin
		select * from user
		where email = email_var;
	end //
delimiter ;

delimiter //
create procedure get_user_salt_hash_with_email(in email_var varchar(255))
	begin
		select user.pasword_salt, user.pasword_hash from user
		where email = email_var;
	end //
delimiter ;

delimiter //
create procedure create_session_row(in userId int,in sessionId int)
	begin
		insert into
        session(user_id, last_use, session_id)
		values(userId, now(), sessionId);
	end //
delimiter ;

delimiter //
create procedure get_user_id_from_session(in sessionId int)
	begin
		select user_id from session
		where session_id = sessionId;
	end //
delimiter ;

alter table session
modify session_id varchar(256) not null;

drop procedure get_user_id_from_session;

delimiter //
create procedure get_user_id_from_session(in sessionId varchar(256))
	begin
		select user_id from session
		where session_id = sessionId;
	end //
delimiter ;

drop procedure get_user_with_session;

delimiter //
create procedure get_user_with_session(in sessionId varchar(256))
	begin
		select * from user
		where id = (
		select session.user_id
		from session
		where session.session_id = session_id_var);
	end //
delimiter ;


drop procedure create_session_row;

delimiter //
create procedure create_session_row(in userId int,in sessionId varchar(256))
	begin
		insert into
        session(user_id, last_use, session_id)
		values(userId, now(), sessionId);
	end //
delimiter ;

drop procedure update_session_last_use;
    delimiter //
create procedure update_session_last_use(in session_id_var varchar(256))
	begin
		update session
		set last_use = now()
		where session_id = session_id_var;
	end //
delimiter ;

delimiter //
create procedure get_flight_by_id(in flightId int)
	begin
		select *
        from flight
        where id = flightId;
	end //
delimiter ;