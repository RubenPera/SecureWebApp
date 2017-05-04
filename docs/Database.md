# Database 

## Tables

### user
- id : int
- name : string
- password_hash : string
- password_salt : string
- email : string
- inholland_miles : int
- paspoort_id : string
- paspoort_valid_date : date
- role : int


### flight
- id : int
- date : date
- price : int
- flight_source : string
- flight_destination : string
- capacity : int
- external_id : int


### booking
- id : int
- user_id : int
- flight_id : int

### session
- session_id : int
- user_id : int
- last_use : timestamp