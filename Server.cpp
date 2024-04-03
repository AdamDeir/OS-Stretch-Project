#include "thread.h"
#include "socketserver.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace Sync;
// global varibles to track clients
std::vector<Socket *> socketVectorTracker;
std::vector<Socket *> inGame;

// class to manage individual client connections
class SocketThread : public Thread
{
private:
    ByteArray userInput; // buffer for storing user input
    bool &endThread;     // boolean to thread termination
    Socket &socket;      // socket reference

public:
    // constructor
    SocketThread(Socket &socket, bool &endThread) : socket(socket), endThread(endThread)
    {
        std::cout << " " << std::endl;
        bool existingClient = false;
        for (int i = 0; i < socketVectorTracker.size(); i++)
        {
            if (socketVectorTracker[i] == &socket)
            { // Compare pointers
                existingClient = true;
                break;
            }
        }
        if (!existingClient)
        {
            std::cout << "New Client Connected" << std::endl;
            socketVectorTracker.push_back(&socket); // Store the pointer

            if (inGame.size() < 2)
            {
                std::cout << "Added Client to Game" << std::endl;
                inGame.push_back(&socket); // Store the pointer
            }
            if (inGame.size() == 2)
            {
                bool senderInGame =false;
                for (int i = 0; i < inGame.size(); i++)
                    {
                        if (&socket == inGame[i])
                        {
                            std::cout << "Sender is in the game" << std::endl;
                            senderInGame = true;
                        }
                    }

                if(senderInGame){
                std::string msg = "Hey mother fucker -- you are player 1";
                socket.Write(msg);
                }else{
                std::string msg = "Hey mother fucker -- you are spectating";
                socket.Write(msg);
                }
            }
            else
            {
                std::cout << "Game Full, Client must spectate" << std::endl;
            }
        }
        else
        {
            std::cout << "Client reconnected" << std::endl;
        }

        // Debug print the connected clients and players in the game
        std::cout << "Printing Connected Clients." << std::endl;
        for (Socket *s : socketVectorTracker)
        {
            std::cout << s << std::endl; // Print the pointer for demonstration
        }

        std::cout << "Printing Players in the current game" << std::endl;
        for (Socket *s : inGame)
        {
            std::cout << s << std::endl; // Print the pointer for demonstration
        }
    }

    virtual ~SocketThread() {} // destructor

    // socket getter
    Socket &GetSocket()
    {
        return socket;
    }

    // main thread function
    virtual long ThreadMain() override
    {
        while (!endThread) // continue running until signaled to stop
        {
            try
            {
                int bytesRead = socket.Read(userInput); // read data from the socket
                if (bytesRead > 0)                      // if data was received
                {
                    std::string response = userInput.ToString(); // convert to string
                    std::cout << "Data received: " << response << " -- From Socket: " << &socket << std::endl;

                    // Forward message to the other client in the game
                    // Iterate through the inGame vector to find the opponent
                    bool senderInGame = false;
                    int oppIndex = 0;
                    for (int i = 0; i < inGame.size(); i++)
                    {
                        if (&socket == inGame[i])
                        {
                            std::cout << "Sender is in the game" << std::endl;
                            senderInGame = true;

                            if (i == 0)
                            {
                                oppIndex = 1;
                            }
                        }
                    }

                    if (senderInGame)
                    {
                        // Check if the current iterator is not pointing to the sender's socket.
                        (*inGame[oppIndex]).Write(response); // Send the message to the opponent.
                        std::cout << "Message sent to opponent: " << inGame[oppIndex] << std::endl;

                        // std::string msg = "Message was sent to opponent";
                        // socket.Write(msg);
                        //  Since there are only two players, we can break after finding the opponent.
                    }else{
                        std::string msg = "Shut the fuck up";
                        socket.Write(msg);
                    }
                }
                else if (bytesRead == 0) // client disconnected
                {
                    std::cout << "Client disconnected.\n";
                    break; // exit
                }
            }
            catch (...)
            { // error handling
                std::cout << "Error or client forceful disconnect.\n";
                break;
            }
        }

        socket.Close(); // close the socket
        return 0;
    }
};

// class for to manage the server and client threads
class ServerThread : public Thread
{
private:
    std::vector<SocketThread *> socketThreads;
    bool endThread = false;
    SocketServer &server;

public:
    // constructor
    ServerThread(SocketServer &server) : server(server) {}

    // destructor
    virtual ~ServerThread()
    {
        for (auto *thread : socketThreads)
        { // clean up allocated socket threads
            delete thread;
        }
    }

    // main thread function
    virtual long ThreadMain() override
    {
        while (!endThread)
        { // continue until signaled to stop
            try
            {
                Socket *newConnection = new Socket(server.Accept());                  // accept new connections
                socketThreads.push_back(new SocketThread(*newConnection, endThread)); // create and manage new socket thread
            }
            catch (...)
            { // handle exceptions
                endThread = true;
                break;
            }
        }

        return 0;
    }
};

// main function to initialize the server
int main()
{
    std::cout << "I am a server.\nPress enter to terminate the server.\n";
    SocketServer server(3000);         // start the server on port 3000
    ServerThread serverThread(server); // create and start the server thread

    std::cin.get(); //  user input to terminate
    std::cout << "Terminating server. Goodbye!\n";
    server.Shutdown(); // shut down the server
}
