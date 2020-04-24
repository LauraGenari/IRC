server: serverTest.cpp
	g++ serverTest.cpp -o server -pthread

client: clientTest.cpp
	g++ clientTest.cpp -o client -pthread