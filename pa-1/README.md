
## Homework2 - programming


#### Group Member

- Xuanhe Zhou
- Zeyu Wang

**TCP_Client:** The program accepts two inputs: the port number and the ip address of the server. I've also enabled the checking of whether the port number is provided and valid. The client for TCP will first open the socket and connect with the server with connect() function. After establishing a successful connection, the user is allowed to input a string of numbers. Then the input digits are sent to the server in string form to request a sum. Then it will continuously read the output from the server using the read() method. The read method will read all input into the buffer from the server and thus implement the character separator for split different responses from the server. After the server finishes sending the response, that is when the response contains only one digit, the client will close the socket.

**TCP_Server:** The server will first open the socket and bind the socket to a port, then using the accept() method it could help the client to establish connection with the server. Then the server will receive the request from the client by calling the read() method and store the request(the number client passed in) in the buffer. Then for the processing part, the server will add all digit of the request number while also checking if the number is valid, if it is a string then it will output cannot compute. Otherwise, it will repeatedly add the digit of the sum together and send a sum of the digit to the client using the send() method repeatedly for every sum it gets. Several memset() is needed for clean the buffer and prepare it for next message storage,

**UDP Client:** The client sends a user-inputted string to the server and receives the result of the computation performed by the server. If the user input is invalid (i.e., it contains non-digit characters), the server sends "Sorry, cannot compute!" back to the client. If the user input is valid, the server computes the sum of the digits in the input string and sends the result back to the client. The server repeatedly computes the sum of the numbers until a single-digit result is obtained and returns each intermediate result to the client. The client then prints each result received from the server.

1.  Create a client socket using the socket() function.
    
2.  Set up the server address structure using the struct sockaddr_in data type and fill it with the server's IP address and port number using the inet_addr() and tons () functions, respectively; they are from the user's input in terminal argv[1], argv[2].
    
3.  Send data to the server using the send to () function.
    
4.  Receive data from the server using the recvfrom() function until you get the signal from the server that the "process" has been completed.
    
5.  Close the client socket using the close() function.

**UDP Server:** The server code listens for incoming connections on a specific port specified by the argv[1]. It creates a UDP socket, binds it to the server address and port, and enters a loop to wait for incoming datagrams. When a datagram is received, it prints the message from the client, checks whether it is all numbers, and sends the appropriate response to the client. The server will stay active but will give the client a message like" bye" to let the client close the connection. It has this two different processes:

1.  If the data is not all numbers, it sends a response "Sorry, cannot compute!" to the client and sends a message to let the client terminate the connection.
    
2.  If the data is all numbers, it adds all the digits of the received number, sends the sum to the client, and keeps adding the digits until the sum is a single digit. Then it sends the final sum to the client and sends a message to let the client terminate the connection.
    

 **Reference：**
 
**TCP:** I've used the skeleton code from socket programming c from Geeks2Geeks and edited it to satisfy our server and client requirements. I've also used StackOverflow when I found problems.

**UDP:** I refered to the socket skeleton from socket demos from the section's slides(https://www.linuxhowtos.org/data/6/client_udp.c, https://www.linuxhowtos.org/data/6/server_udp.c ), and when I face with problems I use google and stackoverflow to help me figure them out.
