all: Client Server

Client : 
	g++ Client.cpp -o Client -lenet

Server :
	g++ Server.cpp -o Server -lenet