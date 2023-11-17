/**
 *
 * @file bind.c
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
#include <ctype.h>
#include "utils.h"
#include "ldap.h"
#include "bind.h"

extern int currentTagPosition;

LdapBind ldap_bind(unsigned char *data, int messageId)
{
    debug(1, "****BIND REQUEST****\n");
    LdapBind bind;
    bind.messageId = messageId;
    bind.version = get_int_value(data);
    bind.name = get_string_value(data);
    bind.authChoice = get_ldap_element_info(data).tagValue;
    return bind;
}

void ldap_bind_response(LdapBind bind, int clientSocket)
{
    debug(1, "****BIND RESPONSE****\n");
    int offset = 0;
    unsigned char buff[MAX_BUFFER_SIZE];
    memset(buff, 0, MAX_BUFFER_SIZE); // clear the buffer

    create_ldap_header(buff, &offset, bind.messageId);

    add_ldap_byte(buff, &offset, LDAP_BIND_RESPONSE);
    add_ldap_byte(buff, &offset, 0x07);

    add_ldap_byte(buff, &offset, ENUMERATED_TYPE);
    add_ldap_byte(buff, &offset, 0x01);

    if (bind.authChoice != SIMPLE_BIND)
    {
        add_ldap_byte(buff, &offset, AUTH_METHOD_NOT_SUPPORTED);
    }
    else if (bind.version != 0x03)
    {
        add_ldap_byte(buff, &offset, PROTOCOL_ERROR);
    }
    else if (strcmp(bind.name, "") != 0)
    {
        add_ldap_byte(buff, &offset, INVALID_DN_SYNTAX);
    }
    else
    {
        add_ldap_byte(buff, &offset, SUCCESS);
    }

    add_ldap_string(buff, &offset, "");

    add_ldap_string(buff, &offset, "");

    buff[LDAP_MSG_LENGTH_OFFSET] = offset - 2; // ldap msg tag, and length
    ldap_send(buff, clientSocket, offset);
    free(bind.name);

    print_hex_message(buff, offset);
}
