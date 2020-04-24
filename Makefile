server: server.cpp
	g++ server.cpp -o server -pthread

client: client.cpp
	g++ client.cpp -o client -pthread

john: 
	./client john 2000

mary:
	./client mary 2000

start:
	./server 2000