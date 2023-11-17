/**
 *
 * @file ldap.c
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *
 */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <ctype.h>
#include "utils.h"
#include "ldap.h"
#include "bind.h"
#include "search.h"

// represents offset pointer to revecied data
int currentTagPosition;

int ldap_handle_request(unsigned char *data, size_t length, int clientSocket, FILE *file)
{
    if (length < 5)
    {
        ldap_notice_of_disconnection(clientSocket);
        printf("Received an unknown or unsupported message.\n");
        return -1;
    }

    LdapElementInfo elementInfo = get_ldap_element_info(data);

    int messageId = get_int_value(data);
    elementInfo = get_ldap_element_info(data);

    switch (elementInfo.tagValue)
    {
    case LDAP_BIND_REQUEST:
        LdapBind bind = ldap_bind(data, messageId);
        ldap_bind_response(bind, clientSocket);
        break;

    case LDAP_SEARCH_REQUEST:
        LdapSearch search = ldap_search(data, messageId);
        print_ldap_search(search);
        ldap_search_response(search, clientSocket, file);
        dispose_ldap_search(search);
        break;

    case LDAP_UNBIND_REQUEST:
        debug(1, "****UNBIND REQUEST****\n");
        return -1;
        break;

    default:
        ldap_notice_of_disconnection(clientSocket);
        debug(1, "Received an unknown or unsupported operation.\n");
        return -1;
    }

    return 0;
}

void ldap_notice_of_disconnection(int clientSocket)
{
    int offset = 0;
    unsigned char buff[MAX_BUFFER_SIZE];
    memset(buff, 0, MAX_BUFFER_SIZE); // clear the buffer

    create_ldap_header(buff, &offset, 0);
    add_ldap_byte(buff, &offset, LDAP_EXTENDED_RESPONSE);
    int extendedResponseOffset = offset;
    add_ldap_byte(buff, &offset, LDAP_PLACEHOLDER);

    add_ldap_byte(buff, &offset, ENUMERATED_TYPE);
    add_ldap_byte(buff, &offset, 1);
    add_ldap_byte(buff, &offset, UNAVAILABLE);

    add_ldap_string(buff, &offset, "");
    add_ldap_string(buff, &offset, "Received an unknown or unsupported message.");
    add_ldap_oid(buff, &offset, "1.3.6.1.4.1.1466.20036");
    buff[LDAP_MSG_LENGTH_OFFSET] = offset - 2; // ldap msg tag, and length
    buff[extendedResponseOffset] = offset - extendedResponseOffset - 1;
    ldap_send(buff, clientSocket, offset);
}

void ldap(int clientSocket, FILE *file)
{
    size_t receivedBytes;
    int recivedDataCode = 0;
    while (recivedDataCode != -1)
    {
        unsigned char *receivedData = ldap_receive(clientSocket, &receivedBytes);
        debug(1, "Received data from client:\n");
        print_hex_message(receivedData, receivedBytes);

        currentTagPosition = 0;
        recivedDataCode = ldap_handle_request(receivedData, receivedBytes, clientSocket, file);
        // Free the allocated memory for receivedData
        free(receivedData);
    }
}