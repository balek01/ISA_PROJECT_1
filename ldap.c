/**
 *
 * @file ldap.c
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  Last modified: Oct 3, 2023
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
#include "ldap.h"

int MAX_BUFFER_SIZE = 2048;

int ldap_parse_request(LdapRequest *request, unsigned char *data, size_t length)
{

    if (data[0] != 0x30)
    {
        return -1;
    }
    request->message_id = data[4];

    if (data[5] == 0x60)
    {
        printf("Bind message received");
    }

    return 0;
}

void ldap(int client_socket)
{
    size_t receivedBytes;
    unsigned char *receivedData = ldap_receive(client_socket, &receivedBytes);
    printHexMessage(receivedData, receivedBytes);
    LdapRequest request;

    if (ldap_parse_request(&request, receivedData, receivedBytes) == -1)
    {
        printf("Error parsing \n");
        free(receivedData);
        return;
    }

    printf("%d", request.message_id);

    // Free the allocated memory for receivedData
    free(receivedData);
}

void printHexMessage(const unsigned char *data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("%02X ", data[i]); // Print each byte in hexadecimal format
    }
    printf("\n");
}

unsigned char *ldap_receive(int client_socket, size_t *receivedBytes)
{
    unsigned char buffer[MAX_BUFFER_SIZE];
    int bytesReceived;

    // Receive data into the buffer using recv
    bytesReceived = recv(client_socket, buffer, sizeof(buffer), 0);

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
