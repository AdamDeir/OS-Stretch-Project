#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "socket.h" 
#include "thread.h" 

using namespace Sync; // Sync namespace for synchronization utilitie

class ClientManager;

// inherits from thread and manages the client's communication with the server
class ServerCommunicator : public Thread {
private:
    Socket& _socket; 
    bool& _terminateFlag; 
    bool _isConnected = false; //connection status

public:
    // constructor initializes the socket and terminate flag
    ServerCommunicator(Socket& socket, bool& terminateFlag)
        : _socket(socket), _terminateFlag(terminateFlag) {}

    // attempts to establish a connection to the server
    void EstablishConnection() {
        //get username from the user in the temrinal before connecting to server
        // std::string clientName;
        // std::cout << "Enter your name (This will be your ID for the session): ";
        // std::getline(std::cin, clientName);

        int attemptCount = 0; // counts the number of connection attempts
        // keep trying to connect until successful, attempt limit reached, or termination requested
        while (!_isConnected && attemptCount < 5 && !_terminateFlag) {
            std::cout << "Trying to connect...";
            try {
                _socket.Open(); // attempt to open the socket connection
                _isConnected = true; // set connection status to true on success
                std::cout << "Connected successfully." << std::endl;
                //SendMessageToServer("ClientName:" + clientName); // Prefix with "ClientName:" to indicate ID message
                
            } catch (...) {
                std::cout << "Connection attempt failed." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5)); // wait 5 seconds before retrying
            }
            ++attemptCount; // increment attempt counter
        }
        
        // if connection could not be established, set the termination flag
        if (!_isConnected) {
            std::cout << "Connection attempts exceeded. Exiting." << std::endl;
            _terminateFlag = true;
        }
    }

    // the main function to be executed by the thread
    virtual long ThreadMain() override {
        EstablishConnection(); // first attempt to establish a connection
        
        // as long as termination is not requested and connection is active, handle server communication
        while (!_terminateFlag && _isConnected) {
            ListenToServer();
            HandleServerCommunication();
        }

        return 0; 
    }

    // handles the interaction with the server
    void HandleServerCommunication() {
        std::string inputMessage; // to store user input
        std::cout << "Enter a message (or enter 'done' to exit): ";
        std::getline(std::cin, inputMessage); // read a message from the user

        // check if the input message is a command to terminate
        if (inputMessage.empty()) {
            std::cout << "Message is empty, nothing sent." << std::endl;
            return;
        } else if (inputMessage == "done") {
            std::cout << "Terminating session." << std::endl;
            _terminateFlag = true;
        } else {
            SendMessageToServer(inputMessage); // send the message to the server  if message isnt empty and message isnt done
        }
    }

    // sends a message to the server and waits for a response
    void SendMessageToServer(const std::string& message) {
        if (_socket.Write(ByteArray(message)) <= 0) {
            std::cout << "Failed to send message. Disconnecting." << std::endl;
            _terminateFlag = true; //  terminate if message sending fails
            return;
        }

        ByteArray response; // store the server response
        _socket.Read(response); // read the server response
        // if no response is received
        
        if (response.ToString().empty()) {
            std::cout << "No response received. Disconnecting." << std::endl;
            _terminateFlag = true;
            return;
        }
        std::cout << "Response from server: " << response.ToString() << std::endl;
        HandleServerCommunication();
    }

    void ListenToServer(){
        ByteArray response; // store the server response
        _socket.Read(response); // read the server response

        if (response.ToString().empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
                std::cout << "Listening again " << std::endl;
                ListenToServer();
        }else if(response.ToString() == "Spectating..."){
            std::cout << "You Are Spectating" << std::endl;
            ListenToServer();
        }
        
        else{
            std::cout << "Response from server: " << response.ToString() << std::endl;
            
        }

        
    }


};


// main function to set up the client and initiate communication
int main() {
    std::cout << "I am a client." << std::endl; 

    Socket socket("127.0.0.1", 3000); // create socket
    bool terminateFlag = false;  

    ServerCommunicator communicator(socket, terminateFlag); // create a servercommunicator instance

    while (!terminateFlag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    }

    try {
        socket.Close(); // close the socket on program termination
        std::cout << "Socket closed successfully." << std::endl;
    } catch (...) {
        std::cerr << "Error closing socket." << std::endl;
    }

    return 0;
}
