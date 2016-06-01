/*
 *  server.c
 *
 *  Ed Jones
 *
 *  Main C file for sudoku program.
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
void serveHTML (int socket);
char *extract (char *message);
void serveAlmond (int socket, char *sudoku);
void serveError (int socket);

#define SIMPLE_SERVER_VERSION 1.0
#define REQUEST_BUFFER_SIZE 1000
#define DEFAULT_PORT 7191
#define NUMBER_OF_PAGES_TO_SERVE 100

int main (int argc, char *argv[]) {
    printf ("************************************\n");
    printf ("Starting sudoku server %f\n", SIMPLE_SERVER_VERSION);
    printf ("Serving sudoku since 2016\n");
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

        if (request[5] != ' ') {
            char *sudoku = request;
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
                serveError (connectionSocket);
            } else {
                serveAlmond (connectionSocket, return_board(&b));
            }
        } else {
            serveHTML (connectionSocket);
        }

        // STEP 4. close the connection after sending the page
        close (connectionSocket);
        numberServed++;
    }

    // close the server connection after we are done
    printf ("** shutting down the server **\n");
    close (serverSocket);

    return EXIT_SUCCESS;
}

void serveHTML (int socket) {

    FILE *input;
    input = fopen("input.html", "r");

    char* message = malloc(sizeof(char)*10000);

    fscanf(input, " %[^\a]s", message);
    assert(fclose(input) == 0);

    printf ("\nSERVING BOARD\n");

    send (socket, message, strlen (message), 0);
    free (message);

}

void serveAlmond (int socket, char* sudoku) {

    FILE *solution;
    solution = fopen("solution.html", "r");

    char *message = malloc(sizeof(char)*10000);

    fscanf(solution, "%[^\a]s", message);
    assert(fclose(solution) == 0);

    const char* header =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n";

    printf ("\nSERVING SOLUTION\n");

    send (socket, header, strlen (header), 0);
    send (socket, sudoku, strlen (sudoku), 0);
    send (socket, message, strlen (message), 0);

    free(message);

}

void serveError (int socket) {
    const char *message =
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
    printf ("\nSERVING ERROR, NO SOLUTION\n");

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