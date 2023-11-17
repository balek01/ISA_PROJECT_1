/**
 *
 * @file bind.h
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *
 */
#ifndef _BIND_H
#define _BIND_H
/**
 * Structure representing an LDAP Bind message.
 *
 * The LdapBind structure is used to represent an LDAP Bind message, containing
 * essential information about the message, including the message ID and the protocol version.
 */
typedef struct
{
    int messageId;  /**< The unique identifier for the LDAP Bind message. */
    int version;    /**< The protocol version associated with the LDAP Bind message. */
    char *name;     /**< The distinguished name associated with the LDAP Bind message. */
    int authChoice; /**<*/
} LdapBind;

enum AuthChoice
{
    SIMPLE_BIND = 0x80,
    SASL_AUTH = 0xA3
};

/**
 * LDAP Bind Operation.
 *
 * Initiates an LDAP bind operation using the provided data and message ID.
 *
 * @param data  A pointer to the data required for the bind operation.
 * @param messageId An integer representing the unique identifier for the LDAP message.
 *
 * @return An LdapBind structure representing the result of the bind operation.
 *
 * The caller is responsible for managing any associated memory. 
 * The 'name' field in the structure is dynamically
 * allocated and should be freed by the caller when no longer needed.
 */
LdapBind ldap_bind(unsigned char *data, int messageId);

/**
 * Send an LDAP Bind response to the client.
 *
 * This function sends an LDAP Bind response to the client over the specified
 * client socket. The response contains information about the Bind operation,
 * including the message ID and result code.
 *
 * @param bind The LdapBind structure representing the Bind request.
 * @param clientSocket The socket for communication with the client.
 */
void ldap_bind_response(LdapBind bind, int clientSocket);

void ldap_send(unsigned char *bufin, int clientSocket, int offset);

#endif