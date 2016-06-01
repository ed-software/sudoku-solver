/*
*  simpleServer.c
*  1917 lab 4
*
*  Richard Buckland 28/01/11, 30/3/14.
*  Licensed under Creative Commons SA-BY-NC 3.0, share freely.
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <unistd.h>
#include "sudoku.h"

int waitForConnection (int serverSocket);
int makeServerSocket (int portno);
void serveHTML (int socket, FILE *in);
char* extract (char *message);
void serveAlmond (int socket, char* sudoku);
void serveError (int socket);

#define SIMPLE_SERVER_VERSION 2.0
#define REQUEST_BUFFER_SIZE 1000
#define DEFAULT_PORT 7191
#define NUMBER_OF_PAGES_TO_SERVE 100
// after serving this many pages the server will halt

int main (int argc, char* argv[]) {

    FILE *in;

    if (argc > 2) {
        fprintf(stderr, "ERROR: too many arguments\n");
        return 1;
    }

    if (argc == 2) {
        in = fopen(argv[1], "r");
        if (in == NULL) {
            fprintf(stderr, "ERROR: could not open \"%s\"\n", argv[1]);
            return 2;
        }
    }

    printf ("************************************\n");
    printf ("Starting simple server %f\n", SIMPLE_SERVER_VERSION);
    printf ("Serving poetry since 2011\n");
    printf ("Access this server at http://localhost:%d/\n", DEFAULT_PORT);
    printf ("************************************\n");

    int serverSocket = makeServerSocket(DEFAULT_PORT);
    char request[REQUEST_BUFFER_SIZE];
    int numberServed = 0;
    while (numberServed < NUMBER_OF_PAGES_TO_SERVE) {
        printf ("*** So far served %d pages ***\n", numberServed);

        // STEP 1. wait for a request to be sent from a web browser,
        // then open a new connection for this conversation
        int connectionSocket = waitForConnection(serverSocket);

        // STEP 2. read the first line of the request
        int bytesRead = recv (connectionSocket, request, sizeof(request) - 1, 0);
        assert (bytesRead >= 0);
        // check that we were able to read some data from the connection

        // echo entire request to the console for debugging
        printf (" *** Received http request ***\n %s\n", request);

        // STEP 3. send the browser a simple html page using http
        printf (" *** Sending http response ***\n");

        if (request[5] != ' ') {
            char* sudoku = request;
            sudoku += 5;
            int i = 0;
            sudoku[81] = 0;

            struct board b;
            int ret;
            // Initialize data structures.
            init_board(&b);

            // Read and solve board.
            read_board(sudoku, &b);
            ret = solve_board(&b, MIN_NUM, MIN_NUM);

            if (! ret) {
                fprintf(stderr, "ERROR: board could not be solved\n");
                serveError (connectionSocket);
            } else {
                serveAlmond (connectionSocket, return_board(&b));
            }


        } else {
            serveHTML (connectionSocket, in);
        }

        // STEP 4. close the connection after sending the page- keep aust beautiful
        close (connectionSocket);
        ++numberServed;



    }

    // close the server connection after we are done- keep aust beautiful
    printf ("** shutting down the server **\n");
    close (serverSocket);

    return EXIT_SUCCESS;
}

void serveHTML (int socket, FILE *in) {


    char* message;

    printf("MESSAGE IS \n\n %s", message);
    fscanf(in, " %[^\a]s", message);
/*

    const char* message =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<!DOCTYPE html>"
    "<head>"
    "<title>Sudoku Solver</title>"
    "</head>"
    "<script src=\"http://188.166.189.211/sudoku.js\"></script>"
    "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://188.166.189.211/style.css\">";
    */

    // echo the http response to the console for debugging purposes
    printf ("VVVV about to send this via http VVVV\n");
    printf ("%s\n", message);
    printf ("^^^^ end of message ^^^^\n");

    // send the http response to the web browser which requested it
    send (socket, message, strlen (message), 0);
}

void serveError (int socket) {
    const char* message =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<!DOCTYPE html>"
    "<head>"
    "<title>Sudoku Solver</title>"
    "</head>"
    "<html>"
    "<body>"
    "Error, puzzle could not be solved."
    "</body>"
    "</html>";

    // echo the http response to the console for debugging purposes
    printf ("VVVV about to send this via http VVVV\n");
    printf ("%s\n", message);
    printf ("^^^^ end of message ^^^^\n");

    // send the http response to the web browser which requested it
    send (socket, message, strlen (message), 0);
}


// start the server listening on the specified port number
int makeServerSocket (int portNumber) {

    // create socket
    int serverSocket = socket (AF_INET, SOCK_STREAM, 0);
    assert (serverSocket >= 0);
    // check there was no error in opening the socket

    // bind the socket to the listening port  (7191 in this case)
    struct sockaddr_in serverAddress;
    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port        = htons (portNumber);

    // tell the server to restart immediately after a previous shutdown
    // even if it looks like the socket is still in use
    // otherwise we might have to wait a little while before rerunning the
    // server once it has stopped
    const int optionValue = 1;
    setsockopt (serverSocket, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof (int));

    int bindSuccess = bind (serverSocket, (struct sockaddr*)&serverAddress, sizeof (serverAddress));

    assert (bindSuccess >= 0);
    // if this assert fails wait a short while to let the operating
    // system clear the port before trying again

    return serverSocket;
}

// wait for a browser to request a connection,
// returns the socket on which the conversation will take place
int waitForConnection (int serverSocket) {

    // listen for a connection
    const int serverMaxBacklog = 10;
    listen (serverSocket, serverMaxBacklog);

    // accept the connection
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof (clientAddress);
    int connectionSocket = accept (serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
    assert (connectionSocket >= 0);
    // check for connection error

    return connectionSocket;
}

// Write the image data for a Mandelbrot tile to the server
void serveAlmond (int socket, char* sudoku) {
    char* message;


    // First send the http response header

    message = "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n";
    printf ("Serving a fresh almond from...\n");
    printf ("%s\n", message);

    assert(write (socket, message, strlen (message)));
    assert(write (socket, sudoku, strlen(sudoku)));

}

/*
this code calls these external networking functions
try to work out what they do from seeing how they are used,
then google them for full details.

recv
close
send
socket
setsockopt
bind
listen
accept
*/