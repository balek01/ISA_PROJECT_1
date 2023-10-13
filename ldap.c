/**
 *
 * @file ldap.c
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  Last modified: Oct 13, 2023
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

// represents offeset pointer to revecied data
int currentTagPosition;

int ldap_parse_request(unsigned char *data, size_t length)
{

    if (data[0] != LDAP_MESSAGE_PREFIX)
    {
        printf("ERROR Received not an ldap message\n");
        return -1;
    }
    if (length < 5)
        return -1;

    unsigned char requestOperation = data[5];

    switch (requestOperation)
    {
    case LDAP_BIND_REQUEST:
        printf("Received a Bind Request.\n");
        ldap_bind(data);
        break;
    case LDAP_SEARCH_REQUEST:
        printf("Received a Search Request.\n");
        // ldap_search();
        break;
    case LDAP_MODIFY_REQUEST:
        printf("Received a Modify Request.\n");
        break;

    case LDAP_UNBIND_REQUEST:
        printf("Received a Unbind Request.\n");
        break;

        // TODO: Implement rest of operations

    default:
        printf("Received an unknown or unsupported operation.\n");
    }

    return 0;
}

void ldap_bind(unsigned char *data)
{
    LdapBind bind;
    // TODO: define as constants
    currentTagPosition = 7;
    bind.version = get_int_value(data);
    // TODO handle error response if version is different from 3

    get_string_value(data);
    printf("Version: %d\n", bind.version);
}

void ldap_response()
{
}

LdapElementInfo get_ldap_element_info(unsigned char *data)
{
    LdapElementInfo elementInfo;
    elementInfo.lengthOfData = data[currentTagPosition + 1];
    elementInfo.start = currentTagPosition + 2;
    elementInfo.end = elementInfo.start + elementInfo.lengthOfData - 1;
    elementInfo.nextTagPosition = currentTagPosition + elementInfo.lengthOfData + 2;
    elementInfo.tagValue = data[currentTagPosition];

    print_ldap_element_info(elementInfo);
    return elementInfo;
}

void print_ldap_element_info(LdapElementInfo elementInfo)
{

    printf("Tag Value: %02X\n", elementInfo.tagValue);
    printf("Length of Data: %d\n", elementInfo.lengthOfData);
    printf("Start: %d\n", elementInfo.start);
    printf("End: %d\n", elementInfo.end);
    printf("New Tag Position: %d\n\n", elementInfo.nextTagPosition);
}

void get_string_value(unsigned char *data)
{
    LdapElementInfo LdapInfo = get_ldap_element_info(data);

    if (LdapInfo.tagValue != OCTET_STRING) // TODO: end correctly
        printf("Unexpected element type");

    for (int i = LdapInfo.start; i <= LdapInfo.end; i++)
    {
        printf("%c", data[i]);
    }
    printf("\n");
    ldap_next_element(LdapInfo);
}

void ldap_next_element(LdapElementInfo LdapInfo)
{
    currentTagPosition = LdapInfo.nextTagPosition;
}

long long get_int_value(unsigned char *data)
{

    LdapElementInfo LdapInfo = get_ldap_element_info(data);

    if (LdapInfo.tagValue != INTEGER) // TODO: end correctly
        printf("Unexpected element type");
    // Extract the integer value
    long long combinedDecimalNumber = 0;

    if (LdapInfo.end != LdapInfo.start)
    {
        for (int i = LdapInfo.start; i <= LdapInfo.end; i++)
        {
            combinedDecimalNumber = combinedDecimalNumber * 256 + data[i];
        }
    }
    else
    {
        combinedDecimalNumber = data[LdapInfo.start];
    }
    ldap_next_element(LdapInfo);
    return combinedDecimalNumber;
}

void print_hex_message(const unsigned char *data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("%02X ", data[i]); // Print each byte in hexadecimal format
    }
    printf("\n");
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

void ldap(int clientSocket)
{
    size_t receivedBytes;

    unsigned char *receivedData = ldap_receive(clientSocket, &receivedBytes);
    print_hex_message(receivedData, receivedBytes);

    if (ldap_parse_request(receivedData, receivedBytes) == -1)
    {
        printf("Error parsing \n");
        free(receivedData);
        return;
    }
    // Free the allocated memory for receivedData
    free(receivedData);
}