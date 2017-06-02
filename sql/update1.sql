alter table user
drop column paspoort_id,
drop column paspoort_valid_date;

insert into user(pasword_hash, pasword_salt, email, inholland_miles, role)
values('6a3b61ad13b5687376f53a7a4834b91b4b4a15b0c7d54d42ecd11bfc7f8cdd77a02418da121f7c46c7ef7ec536e78d26075d33248f9e5e5e1a72cbea77a8e84f9e5095e1ea3f806cc3e62add918f3f35dab6741d0c29345c76bc09537eeb82b5c629473f1cd7eb34f792e64044e040d10a13211f17c3312d5edd274206090556',
'asdf', 'test@test.nl', 200, 0);

drop procedure get_user_salt_hash_with_email;

delimiter //
create procedure get_user_salt_hash_with_email(in email_var varchar(255))
	begin
		select user.id, user.pasword_salt, user.pasword_hash from user
		where email = email_var;
	end //
delimiter ;


-- password = asdfasdf
insert into user(pasword_hash, pasword_salt, email, inholland_miles, role)
values('6363dbabb80cdd6d7cb68bc6ce12a0101fea491279edb363b9cc2e757c482f227a9ee407f3880942c593cadd1f6d898f41a9efd0ab7c8e340535c097813d1d983f8e72a2de4296cb6efe68d1c5146a67f7d4639e7c4041d79e0218aac4079ea9bd3dbd6afce516f1f8125c2a2d04bf8c8512af93444bbc4a34b830f924e8ee5b',
'asdfasdf', 'user2@teamalfa.com', 200, 0);

delimiter //
create procedure get_user_with_id(in userId int)
	begin
		select user.id, user.pasword_salt, user.pasword_hash, user.email, user.inholland_miles, user.role from user
        where user.id = userId;
	end //
delimiter ;


delimiter //
create procedure delete_flight(in flightId int)
	begin
		delete from booking
		where booking.flight_id = flightId;
		delete from flight
		where flight.id = flightId;
	end //
delimiter ;


delimiter //
create procedure set_airMiles_for_userId(in userId int, in airMiles int)
	begin
		update user
		set user.inholland_miles = airMiles
		where user.id = userId;
	end //
delimiter ;


alter table flight
add column external_id int not null unique;

drop procedure get_all_flights;

delimiter //
create procedure get_all_flights()
	begin
		select *
        from flight;
	end //
delimiter ;

delimiter //
create procedure get_all_flights_with_id(in flightId int)
	begin
		select flight.id, flight.date, flight.price, flight.flight_source, flight.flight_destination, flight.capacity, flight.external_id
        from flight
        where flight.id = flightId;
	end //
delimiter ;

delimiter //
create procedure get_all_flights_with_external_id(in externalId int)
	begin
		select flight.id, flight.date, flight.price, flight.flight_source, flight.flight_destination, flight.capacity, flight.external_id
        from flight
        where flight.external_id = externalId;
	end //
delimiter ;

delimiter //
create procedure create_booking_and_update_money(in userId int, in flightId int)
	begin
		update flight
        set flight.capacity = flight.capacity - 1
        where flight.id = flightId;
		update user
		set user.inholland_miles = user.inholland_miles- (
			select flight.price
            from flight
			where flight.id = flightId)
			where user.id = userId;
		insert into booking(user_id, flight_id)
        values (userid, flightId);
	end //
delimiter ;

delimiter //
create procedure cancel_booking(in flightId int)
	begin
		delete from booking
        where booking.flight_id = flightId;
        delete from flight
        where flight.id = flightId;
	end //
delimiter ;

insert into flight(date, price, flight_source, flight_destination, capacity, external_id)
values(now(), 200, 'abc', 'cba', 200, (FLOOR( 1 + RAND( ) *6000 )));

delimiter //
create procedure update_password_for_userId(in userId int, in newhash varchar(255))
	begin
		update user
		set user.pasword_hash  = newhash
		where user.id = userId;
	end //
delimiter ;

drop procedure update_password_for_userId;

delimiter //
create procedure update_password_for_userId(in userId int, in newHash varchar(256), in newSalt varchar(256))
	begin
		update user
		set user.pasword_hash  = newhash,
			user.pasword_salt = newSalt
		where user.id = userId;
	end //
delimiter ;
drop procedure update_password_for_userId;
delimiter //
create procedure update_password_for_userId(in userId int, in newHash varchar(257), in newSalt varchar(257))
	begin
		update user
		set user.pasword_hash  = newhash,
			user.pasword_salt = newSalt
		where user.id = userId;
	end //
delimiter ;

RENAME USER 'root'@'127.0.0.1' TO 'yaboi'@'127.0.0.1';

SET PASSWORD FOR 'yaboi'@'localhost' = PASSWORD('Mypass#12441');


CREATE USER 'alfa'@'localhost'
  IDENTIFIED BY 'gktuktilhylvjgrkrbit#oothitkv764764754';
GRANT ALL ON secure_web_app_database.* TO 'alfa'@'localhost';

INSERT INTO user VALUES (NULL, '67cffa2b2a81173549652265667378fc6e0625d0270478fcea23964216c73266e6c1d24b7360bacdc9f2fc3859cd12b5a582559774032cc75a474acb127f1d7fcb8a92156db1ede5a62fa9f88a74c6575648c42e514dfef7c0ec97806f6adfdfb942ba742b1481708d5f13500a268f83d6663137bc1097a1c9e0079b46d87332', 'd890fa0a6034ab0f1ae82f6a73113a9444f08eb3f90e3849752e9d622352c226562336310807525638cdc33cfabeb741050b5cb01e8ca38ba337b18b6b7acb658001aaa5d40a106e8d154f45cbabc0b98009b7fb5184a1bae8d2ccaeb8a658b185ae6f848f02d3227826bbffe1d94d279c16883a95eca5e93488fda16a783d6a', 'gump@gmail.com', '18200', '1');
