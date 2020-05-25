server: server.cpp
	g++ server.cpp -o server -pthread

client: client.cpp
	g++ client.cpp -o client -pthread

startClient: 
	./client 2000

startServer:
	./server 2000