/**
 *
 * @file ldap.c
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  Last modified: Oct 26, 2023
 *
 */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <ctype.h>
#include "ldap.h"

// represents offset pointer to revecied data
int currentTagPosition;

int ldap_parse_request(unsigned char *data, size_t length, int clientSocket, FILE *file)
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
        ldap_bind_response(bind, clientSocket);
        break;
    case LDAP_SEARCH_REQUEST:
        printf("Received a Search Request.\n");
        LdapSearch search = ldap_search(data, messageId);
        print_ldap_search(search);
        ldap_search_response(search, clientSocket, file);
        dispose_ldap_search(search);
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

void ldap_send(unsigned char *bufin, int clientSocket, int offset)
{
    int bytestx;
    // try to send buffer to connected client
    bytestx = send(clientSocket, bufin, offset, 0);
    if (bytestx < 0)
        perror("ERROR in sendto");
    printf("Data has been sent to connected client.\n");
}

LdapSearch ldap_search(unsigned char *data, int messageId)
{
    printf("\n****SEARCH REQUEST****\n");
    LdapSearch search;
    search.returnCode = SUCCESS;
    search.messageId = messageId;
    search.baseObject = get_string_value(data);
    search.scope = get_int_value(data);
    search.derefAliases = get_int_value(data);
    search.sizeLimit = get_int_value(data);
    search.timeLimit = get_int_value(data);
    search.typesOnly = get_int_value(data);
    search.filter = get_ldap_filter(data, &search);
    return search;
}

void ldap_search_response(LdapSearch search, int clientSocket, FILE *file)
{
    printf("\n****SEARCH RESPONSE****\n");
    int offset = 0;
    unsigned char buff[MAX_BUFFER_SIZE];
    memset(buff, 0, MAX_BUFFER_SIZE); // clear the buffer

    create_ldap_header(buff, &offset, search.messageId);
    if (search.returnCode != SUCCESS)
    {
        ldap_search_res_done(buff, &offset, search.returnCode, clientSocket);
    }
    else
    {
        ldap_send_search_res_entrys(buff, &offset, search.filter, file, clientSocket);
        offset = 0;
        memset(buff, 0, MAX_BUFFER_SIZE); // clear the buffer
        create_ldap_header(buff, &offset, search.messageId);
        ldap_search_res_done(buff, &offset, search.returnCode, clientSocket);
    }
}
void to_lowercase(char *string)
{
    for (int i = 0; string[i] != '\0'; i++)
    {
        string[i] = tolower(string[i]);
    }
}
int get_targeted_column(LdapFilter filter)
{
    to_lowercase(filter.attributeDescription);
    if (strcmp(filter.attributeDescription, "cn") == 0 || strcmp(filter.attributeDescription, "commonname") == 0)
        return COMMON_NAME;

    if (strcmp(filter.attributeDescription, "uid") == 0 || strcmp(filter.attributeDescription, "userid") == 0)
        return UID;

    if (strcmp(filter.attributeDescription, "mail") == 0)
        return MAIL;

    printf("Unknown attribute");
    return -1;
}
void ldap_send_search_res_entrys(unsigned char *buff, int *offset, LdapFilter filter, FILE *file, int clientSocket)
{
    char line[1024];
    char fullLine[1024];
    char *saveptr; // Used to maintain the tokenizer state
    int targetColumn = get_targeted_column(filter);
    rewind(file);

    while (fgets(line, sizeof(line), file))
    {
        strcpy(fullLine, line);
        char *token = strtok_r(line, ";", &saveptr);
        int column = 0;

        while (token != NULL)
        {
            removeEOL(token);
            if (column == targetColumn)
            {
                if (is_token_equal_filter_value(filter, token))
                {
                    token = strtok_r(fullLine, ";", &saveptr);
                    FileLine fl;

                    // Extract commonname
                    if (token != NULL)
                        fl.cn = token;
                    // Ignore "uid" in the middle
                    token = strtok_r(NULL, ";", &saveptr);
                    if (token != NULL)
                        fl.uid = token;
                    // Extract mail
                    token = strtok_r(NULL, ";", &saveptr);
                    if (token != NULL)
                    {
                        removeEOL(token);
                        fl.mail = token;
                    }
                    ldap_send_search_res_entry(buff, offset, fl, clientSocket); // todo SIZE limit
                }
                break;
            }

            token = strtok_r(NULL, ";", &saveptr);
            column++;
        }
    }
}
void removeEOL(char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        if (str[i] == '\r' || str[i] == '\n')
            str[i] = '\0';
    }
}
void ldap_send_search_res_entry(unsigned char *buff, int *offset, FileLine fl, int clientSocket)
{

    int newoffset = (*offset);
    unsigned char newbuff[MAX_BUFFER_SIZE]; // coppy header
    memset(newbuff, 0, MAX_BUFFER_SIZE);    // clear the buffer
    memcpy(newbuff, buff, MAX_BUFFER_SIZE);

    add_ldap_byte(newbuff, &newoffset, LDAP_SEARCH_RESULT_ENTRY);
    int resultLengthOffset = newoffset;
    add_ldap_byte(newbuff, &newoffset, LDAP_PLACEHOLDER);
    add_ldap_string(newbuff, &newoffset, fl.uid);
    add_ldap_byte(newbuff, &newoffset, LDAP_PARTIAL_ATTRIBUTE_LIST);
    int attributeListoffset = newoffset;
    add_ldap_byte(newbuff, &newoffset, LDAP_PLACEHOLDER);
    add_ldap_attribute_list(newbuff, &newoffset, "cn", fl.cn);
    add_ldap_attribute_list(newbuff, &newoffset, "mail", fl.mail);

    newbuff[resultLengthOffset] = newoffset - resultLengthOffset - 1;
    newbuff[attributeListoffset] = newoffset - attributeListoffset - 1;
    newbuff[1] = newoffset - 2;
    ldap_send(newbuff, clientSocket, newoffset);
}
void add_ldap_attribute_list(unsigned char *buff, int *offset, char *type, char *value)
{

    add_ldap_byte(buff, offset, LDAP_PARTIAL_ATTRIBUTE_LIST);
    int partialListOffset = (*offset);
    add_ldap_byte(buff, offset, LDAP_PLACEHOLDER);
    add_ldap_string(buff, offset, type);
    add_ldap_byte(buff, offset, LDAP_PARTIAL_ATTRIBUTE_LIST_VALUE);
    int partialListValueOffset = (*offset);
    add_ldap_byte(buff, offset, LDAP_PLACEHOLDER);
    add_ldap_string(buff, offset, value);
    buff[partialListValueOffset] = (*offset) - partialListValueOffset - 1;
    buff[partialListOffset] = (*offset) - partialListOffset - 1;
};

bool is_token_equal_filter_value(LdapFilter filter, char *token)
{ // TODO: substring not working correctly for multiple *
    bool match = false;

    if (filter.filterType == EQUALITY_MATCH_FILTER && strcmp(filter.attributeValue, token) == 0)
    { // Full match
        match = true;
    }
    else if (filter.substringType == PREFIX && strncmp(filter.attributeValue, token, strlen(filter.attributeValue)) == 0)
    { // Prefix
        match = true;
    }
    else if (filter.substringType == INFIX && strstr(token, filter.attributeValue) != NULL)
    { // Infix
        match = true;
    }
    else if (filter.substringType == POSTFIX)
    { // Postfix
        int tokenLen = strlen(token);
        int valueLen = strlen(filter.attributeValue);

        if (tokenLen >= valueLen && strcmp(filter.attributeValue, token + (tokenLen - valueLen)) == 0)
        {
            match = true;
        }
    }

    return match;
};

void ldap_search_res_done(unsigned char *buff, int *offset, int returnCode, int clientSocket)
{
    add_ldap_byte(buff, offset, LDAP_SEARCH_RESULT_DONE);
    int resultLengthOffset = (*offset);
    add_ldap_byte(buff, offset, LDAP_PLACEHOLDER); // placeholder
    add_ldap_byte(buff, offset, ENUMERATED_TYPE);
    add_ldap_byte(buff, offset, 0x01);
    if (returnCode == SUCCESS)
    {
        add_ldap_byte(buff, offset, SUCCESS);

        add_ldap_string(buff, offset, "");

        add_ldap_string(buff, offset, "");
    }

    if (returnCode == UNSUPORTED_FILTER)
    {
        add_ldap_byte(buff, offset, UNWILLING_TO_PERFORM);

        add_ldap_string(buff, offset, "");

        add_ldap_string(buff, offset, "Usage of unsupported filter.");
    }
    buff[resultLengthOffset] = (*offset) - resultLengthOffset - 1;
    buff[1] = (*offset) - 2;
    ldap_send(buff, clientSocket, *offset);
    print_hex_message(buff, *offset);
}

LdapFilter get_ldap_filter(unsigned char *data, LdapSearch *search)
{
    LdapFilter filter;
    filter.filterType = get_ldap_element_info(data).tagValue;

    if (filter.filterType != EQUALITY_MATCH_FILTER && filter.filterType != SUBSTRING_FILTER)
    { // suported filters
        printf("ERROR unsupported filter %02X \n", filter.filterType);
        search->returnCode = UNSUPORTED_FILTER;
        filter.attributeDescription = NULL;
        filter.attributeValue = NULL;
        return filter;
    }

    filter.attributeDescription = get_string_value(data);

    if (filter.filterType == EQUALITY_MATCH_FILTER)
    {
        filter.attributeValue = get_string_value(data);
    }
    else
    {
        get_ldap_element_info(data);
        filter.substringType = data[currentTagPosition];
        filter.attributeValue = get_string_value(data);
    }

    return filter;
}
void dispose_ldap_search(LdapSearch search)
{
    free(search.filter.attributeDescription);
    free(search.filter.attributeValue);
    free(search.baseObject);
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
    add_ldap_byte(buff, offset, LDAP_PLACEHOLDER); // placeholder value

    add_integer(buff, offset, messageId);
}
void add_integer(unsigned char *buff, int *offset, int value)
{
    int numBytes = 0;
    int temp = value;

    while (temp > 0)
    {
        temp >>= 8; // Shift 'temp' right by 8 bits
        numBytes++;
    }

    add_ldap_byte(buff, offset, INTEGER_TYPE);
    add_ldap_byte(buff, offset, numBytes);

    // Add the integer data bytes (in big-endian byte order)
    for (int i = numBytes; i > 0; i--)
    {
        add_ldap_byte(buff, offset, (value >> ((i - 1) * 8)) & 0xFF); // 0xff mask to show only the lowest byte
    }
}

void add_ldap_byte(unsigned char *buff, int *offset, int value)
{
    buff[*offset] = (unsigned char)value;
    (*offset)++;
}

void add_ldap_string(unsigned char *buff, int *offset, char *string)
{
    int lenght = strlen(string);
    add_ldap_byte(buff, offset, OCTET_STRING_TYPE);
    add_ldap_byte(buff, offset, lenght);
    for (size_t i = 0; i < lenght; i++)
    {
        add_ldap_byte(buff, offset, string[i]);
    }
}

void ldap_bind_response(LdapBind bind, int clientSocket)
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

    add_ldap_string(buff, &offset, "");

    add_ldap_string(buff, &offset, "");

    buff[LDAP_MSG_LENGTH_OFFSET] = offset - 2; // ldap msg tag, and length
    ldap_send(buff, clientSocket, offset);

    print_hex_message(buff, offset);
}

void set_application_type(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue)
{

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
    if (elementInfo.tagValue <= POSTFIX && elementInfo.tagValue >= PREFIX)
    {
        set_universal_type(data, &elementInfo, lengthValue);
    }
    else if (elementInfo.tagValue > 0x3F || elementInfo.tagValue == LDAP_MESSAGE_PREFIX) // universal type
    {
        set_application_type(data, &elementInfo, lengthValue);
    }
    else
    {
        set_universal_type(data, &elementInfo, lengthValue);
    }
    //     print_ldap_element_info(elementInfo);
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
void print_ldap_search(LdapSearch search)
{
    printf("LDAP search print:\n");
    printf("Return code: %d\n", search.returnCode);
    printf("MessageId: %d\n", search.messageId);
    printf("BaseObject: %s\n", search.baseObject);
    printf("Scope: %d\n", search.scope);
    printf("DerefAliases: %d\n", search.derefAliases);
    printf("Size limit: %d\n", search.sizeLimit);
    printf("Time limit: %d\n", search.timeLimit);
    printf("TypesOnly: %d\n", search.typesOnly);
    printf("Filter type: %02X\n", search.filter.filterType);
    printf("Filter attribute description: %s\n", search.filter.attributeDescription);
    printf("Filter attribute value: %s\n", search.filter.attributeValue);
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

void ldap(int clientSocket, FILE *file)
{
    size_t receivedBytes;
    int recivedDataCode = 0;
    while (recivedDataCode != 1)
    {
        unsigned char *receivedData = ldap_receive(clientSocket, &receivedBytes);
        printf("Received data from client.\n");
        print_hex_message(receivedData, receivedBytes);

        currentTagPosition = 0;
        recivedDataCode = ldap_parse_request(receivedData, receivedBytes, clientSocket, file);
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