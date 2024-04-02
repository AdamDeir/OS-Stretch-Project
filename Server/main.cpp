#include <iostream>
#include <enet.h>
int MAX_PLAYER_LIMIT = 32;

int main(int argc, char ** argv){

    //intialize the enet server
    if(enet_initialize() != 0){
        fprintf(stderr, "An error occured while intializing enet. \n");
        return EXIT_FAILURE;
    }
    //handle disconnect
    atexit(enet_deinitialize);


    //enet objects to play with
    ENetAddress address;
    ENetHost* server;
    ENetEvent event;


    //where ever the server is running, allow connections from anywhere
    address.host = ENET_HOST_ANY;
    address.port = 7777;


    //create enet host point called server
    //2nd param is peers to connect or aka player limit
    //3rd is number of channels, 4 & 5 is bandwidth, 0 is no limit
    server = enet_host_create(&address, MAX_PLAYER_LIMIT, 1, 0, 0);

    //check if server was created
    if(server == NULL){
        fprintf(stderr, "An error occured while trying to create enet server host\n");
        return EXIT_FAILURE;
    }



    //GAME LOOP
    //perchance change this to not be a while try and have a way to end it 
    while(true){

        //while the server is running 
        while(enet_host_service(server, &event, 1000) > 0){

            //check the different event types that occur
            switch(event.type){


            //handle new connection from a client and print the info
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %x:%u.\n",
                event.peer -> address.host,
                event.peer -> address.port);
                break;


            //handle new message received from connected client
            //print the data about the received message
            case ENET_EVENT_TYPE_RECEIVE:
                printf ("A packet of length %u containing %s was received from %x:%u on channel %u.\n",
                event.packet -> dataLength,
                event.packet -> data,
                event.peer -> address.host,
                event.peer -> address.port,
                event.channelID);
                break;
        
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%x:%u disconnected.\n",
                event.peer -> address.host,
                event.peer -> address.port);
                break;
                



            }
        }
    }
    //GAME LOOP

    //currently unreachable but it handles disconnecting the server
    enet_host_destroy(server);
    return EXIT_SUCCESS;

}