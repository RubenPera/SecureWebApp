use secure_web_app_database;
alter table session
add session_id int not null;

SET SQL_SAFE_UPDATES = 0;
SET GLOBAL event_scheduler = ON;s

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
delimiter ;