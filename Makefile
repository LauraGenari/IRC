server: server.cpp
	g++ server.cpp -o server -pthread

serverDebug:
	g++ server.cpp -g -o server -pthread
	gdb ./server

client: client.cpp
	g++ client.cpp -o client -pthread

clientDebug:
	g++ client.cpp -g -o clientDebug -pthread
	gdb ./clientDebug

startClient: 
	./client 2000

startServer:
	./server 2000
