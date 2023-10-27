/**
 *
 * @file bind.c
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
#include "utils.h"
#include "ldap.h"
#include "bind.h"

extern int currentTagPosition;

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
