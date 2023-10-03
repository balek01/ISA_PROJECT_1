/**
 *
 * @file tcp.c
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  Last modified: Oct 3, 2023
 *  @bug Ending the server using C-c signal does not close connections properly
 *
 */

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

Conn ParseArgs(int argc, char *const argv[])
{
    int opt;
    Conn conn;
    conn.port = -1;

    while ((opt = getopt(argc, argv, "h:p:m:")) != -1)
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
        fprintf(stderr, "Usage: %s  -p <port> -f <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // TODO:: check if file path exists and file is .csv
    /* if (strcmp(conn.mode, "udp") != 0 && strcmp(conn.mode, "tcp") != 0)
     {
         fprintf(stderr, "Error: Invalid mode. Mode must be udp or tcp.\n");
         exit(EXIT_FAILURE);
     }
     */
    return conn;
}



int CreateSocket()
{
    int server_socket;
    int type = SOCK_STREAM; // tcp
    int family = AF_INET6;  // ipv6

    server_socket = socket(family, type, 0); // create ipv6 tcp socket
    if (server_socket <= 0)
    {
        fprintf(stderr, "ERROR: could not create socket");
        exit(EXIT_FAILURE);
    }
    return server_socket;
}

void BindSocket(int server_socket, Conn conn)
{
    struct sockaddr_in6 server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;      // Listen on all available interfaces
    server_addr.sin6_port = htons(conn.port); // Port number

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Socket binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Server is binded on port %d...\n", conn.port);
}

void Listen(int server_socket, Conn conn)
{
    // Listen for incoming connections
    if (listen(server_socket, MAX_USERS) == -1)
    {
        perror("Socket listening failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", conn.port);
}

void Accept(int server_socket)
{
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

//TODO: when C-c i received end all connections
    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1)
        {
            perror("Accepting connection failed");
            continue;
        }

        // Create a new process to handle the client
        pid_t child_pid = fork();
        if (child_pid == -1)
        {
            perror("Fork failed");
            close(client_socket);
            continue;
        }

        if (child_pid == 0)
        {
            //* Child process
            close(server_socket); // Close the server socket in the child process
            //TODO:: implement ldap logic here
            
            //?just to test connection remove later
            int bytestx;
            char bufin[1024] = "BYE\n";
            bytestx = send(client_socket, bufin, strlen(bufin), 0);
            if (bytestx < 0)
                perror("ERROR in sendto");
            

            close(client_socket);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Parent process
            close(client_socket);

           
        }
    }
}

int main(int argc, char *const argv[])
{

   
    Conn conn = ParseArgs(argc, argv);
    int server_socket = CreateSocket();
    BindSocket(server_socket, conn);
    Listen(server_socket, conn);
    Accept(server_socket);
    // TODO:DELETE this segment
    //? NOTE: too keep server allive
    char input[100];
    printf("main code: ");
    scanf("%s", input);
    fgets(input, sizeof(input), stdin);

    close(server_socket);
    return 1;
}
