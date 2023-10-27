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
#include "utils.h"
#include "ldap.h"
#include "bind.h"
#include "search.h"

// represents offset pointer to revecied data
int currentTagPosition;

int ldap_handle_request(unsigned char *data, size_t length, int clientSocket, FILE *file)
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
        recivedDataCode = ldap_handle_request(receivedData, receivedBytes, clientSocket, file);
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