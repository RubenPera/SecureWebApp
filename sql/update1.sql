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



