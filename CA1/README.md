The server is offering projects numbered 0-9. When a client connects to the server, the server shows a list of the available projects to that client. The users volunteers for a project by entering the command
```
V X
```
where ```X``` is the project number the client is vulunteering for.
When 5 clients are volunteered for a project, the server assigns a port for those 5 client and they are connected to that port using a UDP socket where they broadcast their prices in turns (assigned by the server) which they have 10 seconds for.

How to run the server:
```
./server 8888
```

How to connect a client:
```
./client 127.0.0.1 8888
```