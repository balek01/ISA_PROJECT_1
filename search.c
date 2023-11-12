/**
 *
 * @file search.c
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
#include <ctype.h>
#include "ldap.h"
#include "utils.h"
#include "search.h"

extern int currentTagPosition;
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
        ldap_send_search_res_entrys(buff, &offset, &search, file, clientSocket);
        offset = 0;
        memset(buff, 0, MAX_BUFFER_SIZE); // clear the buffer
        create_ldap_header(buff, &offset, search.messageId);
        ldap_search_res_done(buff, &offset, search.returnCode, clientSocket);
    }
}

void ldap_search_res_done(unsigned char *buff, int *offset, int returnCode, int clientSocket)
{
    add_ldap_byte(buff, offset, LDAP_SEARCH_RESULT_DONE);
    int resultLengthOffset = (*offset);
    add_ldap_byte(buff, offset, LDAP_PLACEHOLDER); // placeholder
    add_ldap_byte(buff, offset, ENUMERATED_TYPE);
    add_ldap_byte(buff, offset, 0x01);

    switch (returnCode)
    {
    case SUCCESS:
        add_ldap_byte(buff, offset, SUCCESS);
        add_ldap_string(buff, offset, "");
        add_ldap_string(buff, offset, "");
        break;
    case UNSUPORTED_FILTER:
        add_ldap_byte(buff, offset, UNWILLING_TO_PERFORM);
        add_ldap_string(buff, offset, "");
        add_ldap_string(buff, offset, "Usage of unsupported filter.");
        break;
    case SIZE_LIMIT_EXCEEDED:
        add_ldap_byte(buff, offset, SIZE_LIMIT_EXCEEDED);
        add_ldap_string(buff, offset, "");
        add_ldap_string(buff, offset, "Size limit exceeded.");
        break;

    default:
        add_ldap_byte(buff, offset, UNWILLING_TO_PERFORM);
        add_ldap_string(buff, offset, "");
        add_ldap_string(buff, offset, "Internal error.");
        break;
    }
    buff[resultLengthOffset] = (*offset) - resultLengthOffset - 1;
    buff[1] = (*offset) - 2;
    ldap_send(buff, clientSocket, *offset);
    print_hex_message(buff, *offset);
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

    printf("ERROR: Unknown ldap filter attribute");
    return -1;
}
void ldap_send_search_res_entrys(unsigned char *buff, int *offset, LdapSearch *search, FILE *file, int clientSocket)
{
    char line[1024];
    char fullLine[1024];
    char *saveptr; // Used to maintain the tokenizer state
    int targetColumn = get_targeted_column(search->filter);
    int numberOfEntries = 0;
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
                if (is_token_equal_filter_value(search->filter, token))
                {
                    if (search->sizeLimit != 0 && numberOfEntries == search->sizeLimit)
                    {
                        search->returnCode = SIZE_LIMIT_EXCEEDED;
                        return;
                    }
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
                    numberOfEntries++;
                    ldap_send_search_res_entry(buff, offset, fl, clientSocket);
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
    newbuff[LDAP_MSG_LENGTH_OFFSET] = newoffset - 2;
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

LdapFilter get_ldap_filter(unsigned char *data, LdapSearch *search)
{
    LdapFilter filter;
    filter.filterType = get_ldap_element_info(data).tagValue;

    if (filter.filterType != EQUALITY_MATCH_FILTER && filter.filterType != SUBSTRING_FILTER)
    { // suported filters
        printf("Received unsupported filter %02X \n", filter.filterType);
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
