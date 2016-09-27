# SillyChatter
Something along the lines of a super unsafe and crashy chat server >_&lt;
###Registration
The user will not be allowed to send any messages until login or registration is done. 
- **Registering:** Client will verify valid user/pass and send register msg to server. Server will verify it and add it to a list of registered users. Upon sucessful registration users will be automatically logged in.
- **Un-Registering:** User have an option to unregister their account. A unregister msg will be sent to server. The username will be freed up. Not sure if this will be implemented yet.

###Connecting
Connection shouldn't be established until user logs in or sucessfully registers. After user register/login the server will either receive the REGISTER or LOGIN msg, and accept a connection. The server will fill info into peers,connStat, as well as record the user address in users. Users is an array of the user data structure.

###Communicating
All communication from client to server all start with an all caps command, followed by it's parameters.
e.g REGISTER myuser mypass

###Disconnecting


###Basic Function Descriptions


###New Function Descriptions
####client.c
####client_registration.c
- **int REGISTER(void):** prompts for a username and password between 4-8 characters and numbers. Send a request to server. Returns 0 if registration is sucessful. 
- **int LOGIN():** promts for username and password. Send a request to server and returns 0 if login sucessful.
- **int UNREGISTER():**
- **int LOGOUT():**

####server.c
- **int MsgHandle(char * msg, int usrIndex):** Takes in a msg and the index of the client that send the msg, pulls it apart and executes it. Doesn't really need to return anything really.

####server_registration.c
- **int REGISTER(char * registration):** takes in the registration info and just sticks it into a file so far.
- **int UNREGISTER(char * username):** Takes in a username and finds it in the file to delete it from registered users list.


###New Function Structures
- **User** Used to store an online user and address. We might not need pass here.
```
struct user{
  char * name;
  char * addr;
}
```
