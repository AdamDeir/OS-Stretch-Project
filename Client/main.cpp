#include <iostream>
#include <enet/enet.h>


int main(int argc, char ** argv){

   if(enet_initialize() !=0){
        fprintf(stderr,"an error occurred while initilizing enet!\n");
        return EXIT_FAILURE;
   }

    atexit(enet_deinitialize);

    ENetHost* client;
    //first param is ip, not needed for client, 2nd -> out going connections, 3rd -> amount of channels, 4 -> incoming bandwidth, 5 -> outgoing bandwidth
    // 0 tells enet we dont want to limit the bandwidth
    client = enet_host_create(NULL, 1, 1, 0,0);


//ensure/check if client was successfully created
if(client == NULL){
    fprintf(stderr, "An error occured while trying to create an Enet client host!\n");
    return EXIT_FAILURE;
}

//holds ip address and port of the server we will be connecting to
ENetAddress address;
ENetEvent event;//hold all events received from server ie data
ENetPeer* peer; //pointer, peer is the server we are connected to, localized in a enet peer, for sending data to
//^what we are connecting to

//change the ip to be what you want connect to, chnage to variable or server when implemtion continues
enet_address_set_host(&address,"127.0.0.1");
address.port =7777; //port to connect to


//initialize the peer
//param 1 = the client, 2nd = reference to address, 3rd = amount of channels, 4th= any data we want to send right away
//connects to the peer 
peer = enet_host_connect(client, &address, 1, 0);

//check if successful
if(peer == NULL){
    fprintf(stderr, "no available peers for initiating an enet connection");
    return EXIT_FAILURE;
}

//check if server/host has contacted back
//allows us to see any events being received by the server
//3rd =  amount of time to wait if we have recieved anything
//basically check if there was an event and check if it was a connect event
//to know if the server has connected to us
if(enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT ){

    puts("connection to 127.0.0.1:7777 succeeded");
}
else{
    enet_peer_reset(peer);
    puts("connection to 127.0.0.1:7777 failed");
    //could loop back to menu here instead of return
    return EXIT_SUCCESS;
}

//GAME LOOP START

//put logic here
//while loop will go here 

while(enet_host_service(client, &event, 1000))//1000 is the delay ie sleep for a second
{
    //switch based on different events that can be received
    switch(event.type){

        //receive any data
        //print off all data from the received 
        case ENET_EVENT_TYPE_RECEIVE:
            printf ("A packet of length %u containing %s was received from %x:%u on channel %u.\n",
			event.packet -> dataLength,
			event.packet -> data,
			event.peer -> address.host,
			event.peer -> address.port,
			event.channelID);
			break;
    }

}
//GAME LOOP END

//disconnect from server
//2nd param is for sending data
enet_peer_disconnect(peer, 0);
//wait until we get a response from the server we just disconnected from
while(enet_host_service(client, &event, 3000) > 0){

    switch(event.type){

        //handle more data received from server
        //destroy the packet because we dont care cuz disconnecting
        case ENET_EVENT_TYPE_RECEIVE:
        enet_packet_destroy(event.packet);
        break;

        //handle a disconnect from the server
        case ENET_EVENT_TYPE_DISCONNECT:
        puts("Disconnection Succeeded");
        break;

    }
}
return EXIT_SUCCESS;



}