/**
 *
 * @file ldap.c
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  Last modified: Oct 15, 2023
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

// represents offset pointer to revecied data
int currentTagPosition;

int ldap_parse_request(unsigned char *data, size_t length, int client_socket)
{
    LdapElementInfo elementInfo = get_ldap_element_info(data);

    if (elementInfo.tagValue != LDAP_MESSAGE_PREFIX)
    {
        printf("ERROR Received not an ldap message\n");
        return -1;
    }
    if (length < 5)
        return -1;

    int messageId = get_int_value(data);
    elementInfo = get_ldap_element_info(data);

    switch (elementInfo.tagValue)
    {
    case LDAP_BIND_REQUEST:
        printf("Received a Bind Request.\n");
        LdapBind bind = ldap_bind(data, messageId);
        ldap_bind_response(bind, client_socket);
        break;
    case LDAP_SEARCH_REQUEST:
        printf("Received a Search Request.\n");
        LdapSearch search = ldap_search(data, messageId);
        dispose_ldap_fiter(search.filter);
        free(search.baseObject);
        break;
    case LDAP_UNBIND_REQUEST:
        printf("Received a Unbind Request.\n");
        return 1;
        break;

        // TODO: Implement rest of operations

    default:
        printf("Received an unknown or unsupported operation.\n");
    }

    return 0;
}

void ldap_send(unsigned char *bufin, int client_socket, int offset)
{
    int bytestx;
    // try to send buffer to connected client
    bytestx = send(client_socket, bufin, offset, 0);
    if (bytestx < 0)
        perror("ERROR in sendto");
    printf("Data has been sent to connected client.\n");
}

LdapSearch ldap_search(unsigned char *data, int messageId)
{
    printf("\n****SEARCH REQUEST****\n");
    LdapSearch search;

    search.messageId = messageId;
    search.baseObject = get_string_value(data);
    printf("Base object Class: %s\n", search.baseObject);

    search.scope = get_int_value(data);
    printf("Scope: %d\n", search.scope);
    search.derefAliases = get_int_value(data);
    search.sizeLimit = get_int_value(data);
    search.timeLimit = get_int_value(data);
    search.typesOnly = get_int_value(data);
    search.filter = get_ldap_filter(data);
    return search;
}

LdapFilter get_ldap_filter(unsigned char *data)
{
    LdapFilter filter;
    filter.filterType = get_ldap_element_info(data).tagValue;
    filter.attributeDescription = get_string_value(data);
    printf("Attribute description %s\n", filter.attributeDescription);
    filter.attributeValue = get_string_value(data);
    printf("Attribute value %s\n", filter.attributeValue);
    return filter;
}
void dispose_ldap_fiter(LdapFilter filter)
{
    free(filter.attributeDescription);
    free(filter.attributeValue);
}

LdapBind ldap_bind(unsigned char *data, int messageId)
{
    printf("\n****BIND REQUEST****\n");
    LdapBind bind;
    bind.messageId = messageId;
    bind.version = get_int_value(data);

    printf("Message id: %d\n", bind.messageId);
    printf("Version: %d\n", bind.version);

    return bind;
}

void create_ldap_header(unsigned char *buff, int *offset, int messageId)
{
    add_ldap_byte(buff, offset, LDAP_MESSAGE_PREFIX);
    add_ldap_byte(buff, offset, 0x0C); // TODO:ACtualy compute this value

    add_ldap_byte(buff, offset, INTEGER_TYPE);
    add_ldap_byte(buff, offset, 1);
    add_ldap_byte(buff, offset, messageId);
}
void add_integer(unsigned char *buff, int *offset, int value)
{
    int numBytes = 0;
    int temp = value;

    while (temp > 0)
    {
        temp >>= 8; // Shift 'temp' right by 8 bits (1 byte)
        numBytes++;
    }

    add_ldap_byte(buff, offset, INTEGER_TYPE);
    add_ldap_byte(buff, offset, numBytes);

    // Add the integer data bytes (in big-endian byte order)
    for (int i = numBytes; i > 0; i--)
    {
        add_ldap_byte(buff, offset, (value >> (i * 8)) & 0xFF); // 0xff mask to show only the lowest byte
    }
}

void add_ldap_byte(unsigned char *buff, int *offset, int value)
{
    buff[*offset] = (unsigned char)value;
    (*offset)++;
}

void ldap_bind_response(LdapBind bind, int client_socket)
{
    printf("\n****BIND RESPONSE****\n");
    int offset = 0;
    unsigned char buff[MAX_BUFFER_SIZE];
    memset(buff, 0, MAX_BUFFER_SIZE); // clear the buffer

    create_ldap_header(buff, &offset, bind.messageId);

    add_ldap_byte(buff, &offset, LDAP_BIND_RESPONSE);
    add_ldap_byte(buff, &offset, 0x07);

    add_ldap_byte(buff, &offset, ENUMERATED_TYPE);
    add_ldap_byte(buff, &offset, 0x01);

    if (bind.version == 0x03)
    {
        add_ldap_byte(buff, &offset, SUCCESS);
    }
    else
    {
        add_ldap_byte(buff, &offset, PROTOCOL_ERROR);
    }

    add_ldap_byte(buff, &offset, OCTET_STRING_TYPE);
    add_ldap_byte(buff, &offset, 0x00);

    add_ldap_byte(buff, &offset, OCTET_STRING_TYPE);
    add_ldap_byte(buff, &offset, 0x00);

    ldap_send(buff, client_socket, offset);
    print_hex_message(buff, offset);
}
void set_application_type(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue)
{

    printf("ITS APPLICATION type \n");

    if (lengthValue >= 0x81 && lengthValue <= 0xFE)
        elementInfo->lengthOfData = lengthValue - 0x80;

    if (lengthValue <= 0x76)
        elementInfo->lengthOfData = 0;

    elementInfo->start = currentTagPosition + 1;
    elementInfo->end = elementInfo->start;
    elementInfo->nextTagPosition = currentTagPosition + elementInfo->lengthOfData + 2;
}

void set_universal_type(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue)
{
    printf("ITS UNIVERSAL type \n");
    if (lengthValue <= 0x76) // maximum of length that can be encoded in one byte
    {

        elementInfo->lengthOfData = data[currentTagPosition + 1];
        elementInfo->start = currentTagPosition + 2;
        elementInfo->end = elementInfo->start + elementInfo->lengthOfData - 1;
        elementInfo->nextTagPosition = currentTagPosition + elementInfo->lengthOfData + 2;
    }
    if (lengthValue >= 0x81 && lengthValue <= 0xFE)
    {
        get_long_length_info(data, elementInfo, lengthValue);
        elementInfo->end = elementInfo->start + elementInfo->lengthOfData - 1;
        elementInfo->nextTagPosition = currentTagPosition + elementInfo->lengthOfData + lengthValue - 0x80 + 2;
    }
}

LdapElementInfo get_ldap_element_info(unsigned char *data)
{
    LdapElementInfo elementInfo;
    elementInfo.tagValue = data[currentTagPosition];

    int lengthValue = data[currentTagPosition + 1];

    if (elementInfo.tagValue <= 0x3F && elementInfo.tagValue != 0x30) // universal type
    {
        set_universal_type(data, &elementInfo, lengthValue);
    }
    else
    {
        set_application_type(data, &elementInfo, lengthValue);
    }

    print_ldap_element_info(elementInfo);
    currentTagPosition = elementInfo.nextTagPosition;
    return elementInfo;
}

void get_long_length_info(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue)
{

    int lengthOfLength = lengthValue - 0x80;
    elementInfo->start = currentTagPosition + 1 + lengthOfLength;
    elementInfo->lengthOfData = 0;
    for (int i = currentTagPosition + 2; i <= elementInfo->start; i++)
    {
        printf("Data[i] %d\n", data[i]);
        elementInfo->lengthOfData = elementInfo->lengthOfData * 256 + (int)data[i];
    }
}

void print_ldap_element_info(LdapElementInfo elementInfo)
{

    printf("Tag Value: %02X\n", elementInfo.tagValue);
    printf("Length of Data: %d\n", elementInfo.lengthOfData);
    printf("Start: %d\n", elementInfo.start);
    printf("End: %d\n", elementInfo.end);
    printf("New Tag Position: %d\n\n", elementInfo.nextTagPosition);
}

char *get_string_value(unsigned char *data)
{
    LdapElementInfo LdapInfo = get_ldap_element_info(data);

    if (LdapInfo.tagValue != OCTET_STRING_TYPE) // TODO: end correctly
        printf("Unexpected element type\n");

    char *extractedString = (char *)malloc(LdapInfo.lengthOfData + 1); // +1 for null terminator
    if (extractedString)
    {
        for (int i = LdapInfo.start, j = 0; i <= LdapInfo.end; i++, j++)
        {
            extractedString[j] = data[i];
        }
        extractedString[LdapInfo.lengthOfData] = '\0'; // Null-terminate the string

        return extractedString;
    }
    else
    {
        printf("Allocation failed.");
        return NULL; // Return an error indicator or handle the error as needed
    }
}

long long get_int_value(unsigned char *data)
{

    LdapElementInfo LdapInfo = get_ldap_element_info(data);

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
        exit(1); // TODO
    }

    // Allocate memory for the received data
    unsigned char *receivedData = (unsigned char *)malloc(bytesReceived);
    if (receivedData == NULL)
    {
        perror("malloc");
        exit(1); // TODO
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
    int recivedDataCode;
    while (recivedDataCode != 1)
    {
        unsigned char *receivedData = ldap_receive(clientSocket, &receivedBytes);
        printf("Received data from client.\n");
        print_hex_message(receivedData, receivedBytes);

        currentTagPosition = 0;
        recivedDataCode = ldap_parse_request(receivedData, receivedBytes, clientSocket);
        if (recivedDataCode == -1)
        {
            printf("Error parsing \n");
            free(receivedData);
            return;
        }
        // Free the allocated memory for receivedData
        free(receivedData);
    }
}