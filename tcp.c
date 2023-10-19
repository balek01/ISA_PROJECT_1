/**
 *
 * @file tcp.c
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  Last modified: Oct 5, 2023
 *  @bug If server is closed using C-c and no client has connected to server yet
 *  debug messages says that client socket was closed instead of welcome socket. But welcome socket was closed correctly.
 *
 */

// fdd3:ce44:9703:0::1/64

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "tcp.h"
// #include "ldap.h"

volatile int ctrl_c_received = false;
int clientSocket, serverSocket;
pid_t pid;

Conn ParseArgs(int argc, char *const argv[])
{
    int opt;
    Conn conn;
    conn.port = DEFAULT_PORT;
    conn.file = NULL;

    while ((opt = getopt(argc, argv, "p:f:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            conn.port = atoi(optarg);
            break;
        case 'f':
            conn.file = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s -p <port> -f <file>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (conn.port == -1 || conn.file == NULL)
    {
        fprintf(stderr, "Usage: %s  -p <port> -f <file> port %d  file %s\n", argv[0], conn.port, conn.file);
        exit(EXIT_FAILURE);
    }

    // TODO: test if this check works properly
    if (conn.port < 0 || conn.port > 65536)
    {
        fprintf(stderr, "Port %d is out of range (0-65536)\n", conn.port);
        exit(EXIT_FAILURE);
    }

    // TODO:: check if file path exists and file is .csv

    return conn;
}

int CreateSocket()
{
    int type = SOCK_STREAM; // tcp
    int family = AF_INET6;  // ipv6

    serverSocket = socket(family, type, 0); // create ipv6 tcp socket
    if (serverSocket <= 0)
    {
        fprintf(stderr, "ERROR: could not create socket");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
    return serverSocket;
}

void BindSocket(Conn conn)
{
    struct sockaddr_in6 server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;      // Listen on all available interfaces
    server_addr.sin6_port = htons(conn.port); // Port number

    if (bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Socket binding failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    printf("Server is binded on port %d...\n", conn.port);
}

void Listen(Conn conn)
{
    // Listen for incoming connections
    if (listen(serverSocket, MAX_USERS) == -1)
    {
        perror("Socket listening failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", conn.port);
}

void handle_sigint(int signum)
{
    printf("Received SIGINT. Closing all connections and exiting...\n");

    // Set the ctrl_c_received flag to indicate that a Ctrl+C signal was received
    ctrl_c_received = 1;

    if (pid == 0)
    {
        printf("Client socket closing..\n");
        if (close(clientSocket) == 0)
            printf("Client socket closed.\n");
    }
    else
    {
        printf("Welcome socket closing..\n");
        if (close(serverSocket) == 0)
            printf("Welcome socket closed.\n");
    }

    exit(EXIT_SUCCESS);
}

void Accept()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (1)
    {
        //? may not even be necessary
        if (ctrl_c_received)
        {
            // The handle_sigint function will handle socket closure, child process termination,
            // then it will exit.
            break;
        }

        clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (clientSocket == -1)
        {
            perror("Accepting connection failed");
            continue;
        }

        // Create a new process to handle the client
        pid = fork();
        if (pid == -1)
        {
            perror("Fork failed");
            close(clientSocket);
            continue;
        }

        if (pid > 0)
        {
            //* Parent process
            close(clientSocket);
        }
        if (pid == 0)
        {
            //* Child process
            close(serverSocket); // Close the server socket in the child process

            printf("New client connection established: socket fd=%d\n", clientSocket);
            ldap(clientSocket);
            printf("Comunication done closing client socket fd=%d\n", clientSocket);

            close(clientSocket);
            exit(EXIT_SUCCESS);
        }
    }
}

int main(int argc, char *const argv[])
{
    signal(SIGINT, handle_sigint);
    Conn conn = ParseArgs(argc, argv);
    serverSocket = CreateSocket();
    BindSocket(conn);
    Listen(conn);
    Accept();
    close(serverSocket);
    return 1;
}
