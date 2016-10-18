# SillyChatter
Something along the lines of a super unsafe and crashy chat server >_&lt;
####https://github.com/wmkwok/SillyChatter
###How to run
- For client, type **make** and then **./client**
- For server, type **make** and then **./server 7295 16**
- the port number must be 7295, as that is what the client is set to connect with.
- the concurrency number can be changed as preferred.

###Known problems
- Registering doesn't check for validity of usernames
- Something is causing any client besides the first to connect, to block, or not having the sent messages being recv'ed by the server until the first client sends something.

###Registration
The user will not be allowed to send any messages until login or registration is done. 
- **Registering:** Client will send register msg to server. Server will add it to a list of registered users. Upon sucessful registration users will be automatically logged in.

###Connecting
Connection is established before user logs in or sucessfully registers, so an ip address will represent anonymity. After user register/login the server will either receive the REGISTER or LOGIN msg and change the address to a username. The server will fill info into peers,connStat, as well as record the user address in users. Users is an array of the user data structure.

###Communicating
All communication from client to server all start with an all caps command, followed by it's parameters.
e.g REGISTER myuser mypass
- REGISTER username password
- LOGIN username password
- LOGOUT
- QUIT
- SEND message
- PSEND user message
- WHOAMI
- ONLINES

###PSEND
PSEND will send a private message to an online user, assuming the user is online. If the user is not online, then a message will be sent back to sender notifying send failure.

###LOGOUT & QUIT
LOGOUT and QUIT will do the same. The server will simply remove the connection and client will recognize the disconnect, display a disconnection message, and quit the program.

###WHOAMI
In case the client doesn't know whether he/she has logged on, they can send this to check whether they are known as their IP address or their username.

###ONLINES
Server will send a list of all online clients here, not including anonymous users.

###Basic Function Descriptions
- **Send_NonBlocking:** Sends a message to target address using the created socket connection.
- **Recv_NonBlocking:** Recieves a message from socket file descriptor.

###New Function Descriptions
####client.c
- **DoReceive():** Main function that would poll for any incoming messages from the server, any incoming typing from stdin, and send any messages to server.

####server.c
- **int MsgHandle(char * msg, int usrIndex):** Takes in a msg and the index of the client that send the msg, pulls it apart and executes it. Doesn't really need to return anything really.

####registration.c
- **int REGISTER(char * registration):** takes in the registration info and just sticks it into a file.
- **int LOGIN(char* login):** takes in login user and pass, attempts to find in registered list, and returns whether logged on or not.

###New Function Structures
- **User** Used to store an online user and address. We might not need pass here.
```
struct user{
  char * name;
  char * addr;
}
```
