/**
 *
 * @file tcp.c
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  @bug If server is closed using C-c and no client has connected to server yet
 *  debug messages says that client socket was closed instead of welcome socket. But welcome socket was closed correctly.
 *
 */


#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "utils.h"
#include "tcp.h"
#include "ldap.h"

volatile int ctrl_c_received = false;
int clientSocket, serverSocket;
FILE *filePtr;
pid_t pid;

Conn ParseArgs(int argc, char *const argv[])
{
    int opt;
    Conn conn;
    conn.port = DEFAULT_PORT;
    conn.filePath = NULL;

    while ((opt = getopt(argc, argv, "p:f:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            conn.port = atoi(optarg);
            break;
        case 'f':
            conn.filePath = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s -p <port> -f <file>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (conn.port == -1 || conn.filePath == NULL)
    {
        fprintf(stderr, "Usage: %s  -p <port> -f <file> port %d  filePath %s\n", argv[0], conn.port, conn.filePath);
        exit(EXIT_FAILURE);
    }

    if (conn.port < 0 || conn.port > 65536)
    {
        fprintf(stderr, "Port %d is out of range (0-65536)\n", conn.port);
        exit(EXIT_FAILURE);
    }

    conn.filePtr = fopen(conn.filePath, "r");
    filePtr = conn.filePtr;
    if (conn.filePtr == NULL)
    {
        fprintf(stderr, "Failed to open file %s\n", conn.filePath);
        exit(EXIT_FAILURE);
    }

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
    debug(1, "Server is binded on port %d...\n", conn.port);
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

    debug(1, "Server is listening on port %d...\n", conn.port);
}

void handle_sigint(int signum)
{
    debug(1, "Received SIGINT. Closing all connections and exiting...\n");

    // Set the ctrl_c_received flag to indicate that a Ctrl+C signal was received
    ctrl_c_received = 1;

    if (pid == 0)
    {
       debug(1, "Client socket closing..\n");
        if (close(clientSocket) == 0)
           debug(1, "Client socket closed.\n");
    }
    else
    {
        debug(1, "Welcome socket closing..\n");
        if (close(serverSocket) == 0)
           debug(1, "Welcome socket closed.\n");
    }
    if (fclose(filePtr) == 0)
       debug(1, "file ptr closed.\n");

    exit(EXIT_SUCCESS);
}

void Accept(Conn conn)
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
            pid = getpid();
            perror("Fork failed");
            if (close(clientSocket) == -1)
                printf("Unable to close socket. %d\n", (int)pid);
            continue;
        }

        if (pid > 0)
        {
            pid = getpid();
            //* Parent process
            if (close(clientSocket) == -1)
                printf("Unable to close socket. %d\n", (int)pid);
        }
        if (pid == 0)
        {
            pid = getpid();
            //* Child process
            if (close(serverSocket) == -1)
                printf("Unable to close socket. %d\n", (int)pid); // Close the server socket in the child process

            debug(1, "New client connection established: socket fd=%d\n", clientSocket);
            ldap(clientSocket, conn.filePtr);
            debug(1,"Comunication done closing client socket fd=%d\n", clientSocket);

            if (close(clientSocket) == -1)
                printf("Unable to close socket. %d\n", (int)pid);

            exit(EXIT_SUCCESS);
        }
    }
}

void ldap_send(unsigned char *bufin, int clientSocket, int offset)
{
    int bytestx;
    // try to send buffer to connected client
    bytestx = send(clientSocket, bufin, offset, 0);
    if (bytestx < 0)
        perror("ERROR in sendto");
    debug(1,"Data has been sent to connected client:\n");
}

unsigned char *ldap_receive(int clientSocket, size_t *receivedBytes)
{
    unsigned char buffer[MAX_BUFFER_SIZE];
    int bytesReceived;

    // Receive data into the buffer using recv
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived < 0)
    {
        perror("recv");
        exit(1);
    }

    // Allocate memory for the received data
    unsigned char *receivedData = (unsigned char *)malloc(bytesReceived);
    if (receivedData == NULL)
    {
        perror("malloc");
        exit(1);
    }

    // Copy the received data to the allocated array
    memcpy(receivedData, buffer, bytesReceived);

    // Set the receivedBytes to the actual number of received bytes
    *receivedBytes = bytesReceived;

    return receivedData;
}

int main(int argc, char *const argv[])
{
    signal(SIGINT, handle_sigint);
    Conn conn = ParseArgs(argc, argv);
    serverSocket = CreateSocket();
    BindSocket(conn);
    Listen(conn);
    Accept(conn);
    fclose(conn.filePtr);
    close(serverSocket);
    return 1;
}
