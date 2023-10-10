/**
 *
 * @file ldap.h
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  Last modified: Oct 3, 2023
 *
 *
 */

#ifndef LDAP_H
#define LDAP_H

extern int MAX_BUFFER_SIZE;

enum ProtocolOp
{
    BIND_REQUEST,
    BIND_RESPONSE,
    UNBIND_REQUEST,
    SEARCH_REQUEST,
    SEARCH_RESULT_ENTRY,
    SEARCH_RESULT_DONE,
};

enum ResultCode
{
    SUCCESS = 0,
    OPPERATION_ERROR = 1,
    TIME_LIMIT_EXCEEDED = 3,
    SIZE_LIMIT_EXCEEDED = 4,
};

typedef struct
{
    int message_id;
    enum ProtocolOp operation;
    unsigned char baseObject[255];
} LdapRequest;

void ldap(int client_socket);
unsigned char *ldap_receive(int client_socket, size_t *receivedBytes);
void printHexMessage(const unsigned char *data, size_t length);
int ldap_parse_request(LdapRequest *request, unsigned char *data, size_t length);

#endif