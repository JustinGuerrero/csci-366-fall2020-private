//
// Created by carson on 5/20/20.
//

#include "stdio.h"
#include "stdlib.h"
#include "server.h"
#include "char_buff.h"
#include "game.h"
#include "repl.h"
#include "pthread.h"
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h>    //inet_addr
#include<unistd.h>    //write
#include<netdb.h>
#include<netinet/in.h>
#include<sys/types.h>
void *connection_handler(void*);
int socket_desc, client_sock, c;
struct sockaddr_in server, client;
pthread_t thread_id;

static game_server *SERVER;

void init_server() {
    if (SERVER == NULL) {
        SERVER = calloc(1, sizeof(struct game_server));
    } else {
        printf("Server already started");
    }
}

int handle_client_connect(int player) {
    // STEP 9 - This is the big one: you will need to re-implement the REPL code from
    // the repl.c file, but with a twist: you need to make sure that a player only
    // fires when the game is initialized and it is there turn.  They can broadcast
    // a message whenever, but they can't just shoot unless it is their turn.
    //
    // The commands will obviously not take a player argument, as with the system level
    // REPL, but they will be similar: load, fire, etc.
    //
    // You must broadcast informative messages after each shot (HIT! or MISS!) and let
    // the player print out their current board state at any time.
    //
    // This function will end up looking a lot like repl_execute_command, except you will
    // be working against network sockets rather than standard out, and you will need
    // to coordinate turns via the game::status field.
}

void server_broadcast(char_buff *msg) {
    // send message to all players
}

int run_server() {

    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1)
    {
        printf("Could not create socket\n");
    }

    struct sockaddr_in connecting_to;

    // google.com
    connecting_to.sin_addr.s_addr = inet_addr("216.58.193.78");
    connecting_to.sin_family = AF_INET;
    // we have to deal with endian-ness
    connecting_to.sin_port = htons(80);

    //Connect to remote server
    if (connect(socket_fd ,
            // we cast here because sockaddr_in
            // is structurally compatible
            // with sockaddr so it works out.
            // Insane, but this is the
            // recommended approach o_O
                (const struct sockaddr *) &connecting_to,
                sizeof(connecting_to)) < 0)
    {
        puts("Could not connect!\n");
    } else {
        puts("Connected!\n");
    }
        while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
            printf("Connection accepted\n");

            if (pthread_create(&SERVER->server_thread, NULL, handle_client_connect, (void *) &client_sock) < 0) {
                printf("Could not create thread\n");
                return 1;
            }
        //Now join the thread , so that we dont terminate before the thread
            pthread_join( SERVER->server_thread , NULL);
            printf("Handler assigned\n");
        }

    //Send some data
    char * message = "GET / HTTP/1.1\r\n\r\n";
    if( send(socket_fd , message ,
             strlen(message) , 0) < 0)
    {
        puts("Send failed");
    } else {
        puts("Data Sent!\n");

        //Receive some data
        char buffer[2000];
        if( recv(socket_fd, buffer , 2000 , 0) < 0)
        {
            puts("Recieve failed");
        } else {
            puts("Reply received\n");
            puts(buffer);
        }
    }
}
//
//    listen(socket_desc , 3);
//
//    //Accept and incoming connection
//    printf("Waiting for incoming connections...\n");
//    c = sizeof(struct sockaddr_in);
//
//    //Accept and incoming connection
//    printf("Waiting for incoming connections...\n");
//    c = sizeof(struct sockaddr_in);
//
//    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
//    {
//        printf("Connection accepted\n");
//
//        if( pthread_create( &SERVER->server_thread , NULL ,  handle_client_connect , (void*) &client_sock) < 0)
//        {
//            printf("Could not create thread\n");
//            return 1;
//        }
//
//        //Now join the thread , so that we dont terminate before the thread
//        pthread_join( SERVER->player_threads , NULL);
//        printf("Handler assigned\n");
//    }
//
//    if (client_sock < 0)
//    {
//        printf("accept failed\n");
//        return 1;
//    }


    // STEP 8 - implement the server code to put this on the network.
    // Here you will need to initalize a server socket and wait for incoming connections.
    //
    // When a connection occurs, store the corresponding new client socket in the SERVER.player_sockets array
    // as the corresponding player position.
    //
    // You will then create a thread running handle_client_connect, passing the player number out
    // so they can interact with the server asynchronously


int server_start() {
//    init_server();
//    pthread_create(&SERVER->server_thread, NULL, (void *) run_server, &client_sock);
//
//}
//     STEP 7 - using a pthread, run the run_server() function asynchronously, so you can still
//     interact with the game via the command line REPL

    int server_socket_fd = socket(AF_INET,
                                  SOCK_STREAM,
                                  IPPROTO_TCP);
    if (server_socket_fd == -1) {
        printf("Could not create socket\n");
    }

    int yes = 1;
    setsockopt(server_socket_fd,
               SOL_SOCKET,
               SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in server;

    // fill out the socket information
    server.sin_family = AF_INET;
    // bind the socket on all available interfaces
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9876);

    int request = 0;
    if (bind(server_socket_fd,
            // Again with the cast
             (struct sockaddr *) &server,
             sizeof(server)) < 0) {
        puts("Bind failed");
    } else {
        puts("Bind worked!");
        listen(server_socket_fd, 88);

        //Accept an incoming connection
        puts("Waiting for incoming connections...");


        struct sockaddr_in client;
        socklen_t size_from_connect;
        int client_socket_fd;
        int request_count = 0;
        while ((client_socket_fd = accept(server_socket_fd,
                                          (struct sockaddr *) &client,
                                          &size_from_connect)) > 0) {
            char raw_buffer[2000];
            char_buff *input_buffer = cb_create(2000);
            char_buff *output_buffer = cb_create(2000);

            int read_size;
            cb_append(output_buffer, "\nbattleBit (? for help) > ");
            cb_write(client_socket_fd, output_buffer);

            while((read_size = recv(client_socket_fd, raw_buffer, 2000, 0))>0){
                cb_reset(output_buffer);
                cb_reset(input_buffer);
                if(read_size>0){
                    raw_buffer[read_size] = '\0';

                    cb_append_int(input_buffer, raw_buffer);

                    char *command = cb_tokenize(input_buffer, " \r\n");
                    if(strcmp(command, "help") == 0){
                        cb_append(output_buffer, "a useful message here");
                        cb_append(output_buffer, command);
                        cb_write(client_socket_fd, output_buffer);

                    }else if(strcmp(command, "quit")==0){
                        close(client_socket_fd);

                    }else if(command != NULL){
                        cb_append(output_buffer, "command was : ");
                        cb_append(output_buffer, command);
                        cb_write(client_socket_fd, output_buffer);
                    }
                    cb_reset(output_buffer);
                    cb_append(output_buffer, "\nbattleBit (? for help) > ");
                    cb_write(client_socket_fd, output_buffer);
                }
            }

        }
    }
    //Create server socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket\n");
        return 0;
    }
    printf("Server Socket Created\n");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8000);

    //Bind the port to the server socket
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("bind failed\n");
        return 0;
    }
    printf("bind done\n");
    return 1;
}

