# Security Summary

## 24 Deadly sins

### Web applications Sins
- SQL injection, validate user input
- cross site scripting, validate user input 
- Buffer overruns, validate user input, user kore_buf
- Magic Urls, preditable cookies and hidden form fields, if in doubt about how to do something ask colleagues

### Implementation Sins
- Integer overflows, 
- Catching exceptions, check if data is valid before using
- Command injection, is handled by database
- Failure to handle errors correctly, on error give error http
- Information leakage, every form should have only one error message, fixed length for page feedback
- Race conditions, partly handled by database, use mutex when needed
- Poor usability, create a logical ux
- Not updating easily, resetting passwords
- Executing code with too much privilege, handled by database user accounts
- Failure to protect stored data, secure the database properly, 
- The sins of mobile code, create a logical ux

### Cryptographic Sins
- Use of weak password-based systems, use a strong password rule
- Weak random numbers, use csprng for generating secure random numbers
- Using cryptography incorrectly, use AES 265 bit encryption only when needed

### Networking Sins
- Failing to protect traffic, use https only
- Improper use of PKI, especially ssl, use https only
- Trusting network name resolution, ???(ask Lando)


## Database security
- 3 accounts
    - website_user
        - can only read values
    - website_admin
        - can read values
        - can write user values
    - admin
        - can do all
- No code injection
- Deadlock prevention
    - set timeout

