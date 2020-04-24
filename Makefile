server: serverTest.cpp
	g++ server.cpp -o server -pthread

client: clientTest.cpp
	g++ client.cpp -o client -pthread
