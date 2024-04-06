#include "thread.h"
#include "socketserver.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <string> // Add missing include statement
#include <cctype>
#include <locale>

using namespace Sync;
// global varibles to track clients
std::vector<Socket *> socketVectorTracker;
std::vector<Socket *> inGame;
std::vector<Socket *> spectatorVectator; 
std::vector<Socket *> completedPlayers;
bool gameOverFlag = false;


// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

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
                std::string msg = "Server asks: Ready to start?\n";
                socket.Write(msg);
                }else{
                std::string msg = "Spectating...\n";
                spectatorVectator.push_back(&socket); // Store the pointer
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
                    std::string compareResponse = response;
                    trim(compareResponse);
                    std::cout<<response<<std::endl;

                    if (compareResponse.compare("GUESSED_IT") == 0){
                        gameOverFlag = true;
                        std::cout <<"in the gussed it loop"<< std::endl;
                    // Iterate through the inGame vector to find the opponent
                    for (int i = 0; i < inGame.size(); i++) {
                        if (&socket != inGame[i]) { // Find the other player
                            std::string winMsg = "PLAYERWIN\n";//write to the player that won
                            completedPlayers.push_back(&(*inGame[i])); 
                            (*inGame[i]).Write(winMsg);
                            std::cout << "Winning message sent to the opponent: " << inGame[i] << std::endl;
                            // Optionally, handle game over logic here
                            // Since message is sent, no need to continue the loop
                        } else { //write to the client that lost
                            std::string loseMsg = "PLAYERLOSE\n";
                            completedPlayers.push_back(&(*inGame[i])); 
                            (*inGame[i]).Write(loseMsg);
                            std::cout << "Losing message sent to the opponent: " << inGame[i] << std::endl;
                        }
                    }
                }else{
                    std::cout <<"did not equal GUESSED_IT"<< std::endl;
                }
                    // Forward message to the other client in the game
                    // Iterate through the inGame vector to find the opponent

                    if(!gameOverFlag){
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
                        std::string msg = "Shut the fuck up\n";
                        socket.Write(msg);
                    }
                }else{
                    std::cout << "GAME IS OVER !!!!" <<std::endl;
                    // for (int i = 0; i < inGame.size(); i++) {
                    //     if (&socket != inGame[i]) { // Find the other player
                    //         std::string winMsg = "PLAYERWIN\n";//write to the player that won
                    //         (*inGame[i]).Write(winMsg);
                    //         std::cout << "Winning message sent to the opponent: " << inGame[i] << std::endl;
                    //         // Optionally, handle game over logic here
                    //         // Since message is sent, no need to continue the loop
                    //     }else{//write to the client that lost
                    //         std::string loseMsg = "PLAYERLOSE\n";
                    //         (*inGame[i]).Write(loseMsg);
                    //         std::cout << "Losing message sent to the opponent: " << inGame[i] << std::endl;
                    //     }
                    // }


                }

                }
                else if (bytesRead == 0) // client disconnected
                {
                    gameOverFlag = false;
                    std::cout << "Client disconnected.\n";
                    for(int i = 0; i < inGame.size(); i++){
                        if(&socket == inGame[i]){
                        std::cout << "Client was in the game.\n";
                        if (!spectatorVectator.empty()){
                        inGame[i] = spectatorVectator.front();
                        for (const auto& socket : spectatorVectator)
                        {
                            std::cout << "Socket: " << socket << std::endl;
                        }
                        std::string msg = "You are now in the game!\n";
                        (*inGame[i]).Write(msg);

                        spectatorVectator.erase(spectatorVectator.begin());
                        std::cout << "Added a spectator to the game: " << spectatorVectator.front() <<std::endl;

                        std::cout<<"Checking if we need to start the game"<<std::endl;

                        bool transitionFlag = false;

                        for (int i = 0; i < inGame.size(); i++) {
                            for (int j = 0; j < completedPlayers.size(); j++) {
                                if (inGame[i] == completedPlayers[j]) {
                                    transitionFlag = true;
                                    break;
                                }
                            }
                            if (transitionFlag) {
                                break;
                            }
                        }            

                        if (inGame.size() == 2 && !transitionFlag)      
                        {
                            std::cout<<"enough players to start new game"<<std::endl;
                            std::string restartMsg = "Server asks: Ready to start?\n";
                            (*inGame[i]).Write(restartMsg);
                        }else{
                            std::cout<<"not enough players to start"<< std::endl;
                        }
                        } else{

                            inGame.erase(inGame.begin() + i);
                            std::cout << "Waiting for more players to join " <<std::endl;

                        }
                        }

                    }
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
