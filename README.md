This is the first computer assignment of my Operating Systems course in the Fall semester 2020 at University of Tehran.

This project is a client-server model implementation of an aucton using Socket Programming in C.
The server is offering projects numbered 0-9. When a client connects to the server, the server shows a list of the available projects to that client (using a TCP socket) so the client can volunteer for a project by entering the command
```
V X
```
where ```X``` is the project number.
When 3 clients are volunteered for a project, the server assigns a port for those 3 clients and they are connected to that port using a UDP socket where they broadcast their prices in turns (assigned by the server) which each have 10 seconds to do. 
When all the clients have broadcasted their prices or the time limit is finished, the winner of the auction (the client who offered the lowest price) will send a message to the server introducing himself/herself as the winner. 

How to run the server:
```
./server 8888
```

How to connect a client:
```
./client 127.0.0.1 8888
```

Please contact me if you need help on how to test/run this. 
