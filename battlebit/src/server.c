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
    struct game *gameon = game_get_current();
    unsigned long long hits = gameon->players[player].hits;
    char raw_buffer[2000];
    char_buff *input_buffer = cb_create(2000);
    char_buff *output_buffer = cb_create(2000);
    int opponent = (player +1 ) % 2;
    int read_size;
    int playerSocket = SERVER->player_sockets[player];
    cb_append(output_buffer, "\nbattleBit (? for help) > ");
    cb_write(playerSocket, output_buffer);
    while ((read_size = recv(playerSocket, raw_buffer, 2000, 0)) > 0) {
        cb_reset(output_buffer);
        cb_reset(input_buffer);
        if (read_size > 0) {
            raw_buffer[read_size] = '\0'; //null terminate read

            // append to input buffer
            cb_append(input_buffer, raw_buffer);

            char *command = cb_tokenize(input_buffer, " \r\n");

            if (command) {
                char* arg1 = cb_next_token(input_buffer);
                char* arg2 = cb_next_token(input_buffer);

                if (strcmp(command, "?") == 0) {
                    // create output
                    cb_append(output_buffer, "? - show help\n");
                    cb_append(output_buffer,"load <string> - load a ship layout file for the given player\n");
                    cb_append(output_buffer,"show - shows the board for the given player\n");
                    cb_append(output_buffer,"fire [0-7] [0-7] - fire at the given position\n");
                    cb_append(output_buffer,"say <string> - Send the string to all players as part of a chat\n");
                    cb_append(output_buffer,"reset - reset the game\n");
                    cb_append(output_buffer,"server - start the server\n");
                    cb_append(output_buffer,"exit - quit the server\n");
                    //cb_append(output_buffer, command);
                    //out put it
                    cb_write(playerSocket, output_buffer);
                } else if (strcmp(command, "exit") == 0) {
                    cb_append(output_buffer, "Goodbye!\n");
                    cb_write(playerSocket, output_buffer);
                    close(playerSocket);
                }else if (strcmp(command, "show") == 0) {
                    repl_print_board(game_get_current(), player, output_buffer);
                    cb_write(playerSocket, output_buffer);
                } else if (strcmp(command, "reset") == 0) {
                    cb_append(output_buffer, "the game has been reset by player ");
                    cb_append_int(output_buffer, player);
                    cb_append(output_buffer, "\n");
                    game_init();
                    server_broadcast(output_buffer);
                } else if (strcmp(command, "load") == 0) {
                    game_load_board(game_get_current(), player, arg1);
                    cb_append(output_buffer, "Player ");
                    cb_append_int(output_buffer, player);
                    cb_append(output_buffer, " has loaded the board\n");
                    //cb_write(playerSocket,output_buffer);
                    server_broadcast(output_buffer);
                } else if (strcmp(command, "fire") == 0) {
                    game_fire(game_get_current(), player, atoi(arg1), atoi(arg2));
                    cb_append(output_buffer, "player ");
                    cb_append_int(output_buffer, player);
                    cb_append(output_buffer, " has fired at ");
                    cb_append_int(output_buffer, atoi(arg1));
                    cb_append(output_buffer, " ");
                    cb_append_int(output_buffer, atoi(arg2));
                   // server_broadcast(output_buffer);
                    if(gameon->players[player].hits != hits){
                        cb_append(output_buffer, " and it's a HIT\n");
                        server_broadcast(output_buffer);
                        if(gameon->players[opponent].ships == 0){
                            cb_append(output_buffer, "Player ");
                            cb_append_int(output_buffer, player);
                            cb_append(output_buffer, " has won! Game will be restarted. Load to continue or type 'exit' to quit");
                            server_broadcast(output_buffer);
                            game_init();
                        }
                    }else{
                        cb_append(output_buffer, " and it's a MISS\n");
                        server_broadcast(output_buffer);
                    }
                } else if (strcmp(command, "say") == 0) {
                    char_buff * tmp = cb_create(1000);
                    cb_append(tmp, raw_buffer + 4);
                    server_broadcast(tmp);
                    cb_free(tmp);
                } else {
                    cb_append(output_buffer,"Unknown Command: ");
                    cb_append(output_buffer, command);
                    cb_append(output_buffer, "\n");
                    cb_write(playerSocket, output_buffer);
                }

                cb_reset(output_buffer);
                cb_append(output_buffer, "\nbattleBut (? for help) > ");
                cb_write(playerSocket, output_buffer);
            }
        }
    }

}

void server_broadcast(char_buff *msg) {
    // send message to all players
    cb_write(SERVER->player_sockets[0], msg);
    cb_write(SERVER->player_sockets[1], msg);



}

int run_server() {
    // STEP 8 - implement the server code to put this on the network.
    // Here you will need to initalize a server socket and wait for incoming connections.
    // When a connection occurs, store the corresponding new client socket in the SERVER.player_sockets array
    // as the corresponding player position.
    // You will then create a thread running handle_client_connect, passing the player number out
    // so they can interact with the server asynchronously

    // creates socket
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_fd == -1) {
        printf("Can't create socket\n");
    }

    //resuse port
    int yes = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in server;

    //socket information
    server.sin_family = AF_INET;

    // bind socket
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9876);

    int request = 0;
    if ( bind(server_socket_fd, (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("BIND FAILED\n");
    } else {
        puts("BIND WORKED\n");
        listen(server_socket_fd, 3);
    }

    puts("WAITING FOR CONNECTION\n");

    // client socket descrption
    struct sockaddr_in client;
    socklen_t size_from_connect;
    int client_socket_fd;
    int request_count = 0;

    while((client_socket_fd = accept(server_socket_fd,
                                     (struct sockaddr *) &client, &size_from_connect)) > 0) {

        //int opponent = (player + 1) % 2;

        SERVER->player_sockets[request_count] = client_socket_fd;


        pthread_create(&SERVER->player_threads[request_count], NULL, handle_client_connect, request_count);

        request_count++;

    }


}

int server_start() {
    // STEP 7 - using a pthread, run the run_server() function asynchronously, so you can still
    // interact with the game via the command line REPL
    init_server();
    pthread_create(&SERVER->server_thread, NULL, run_server, NULL);


}