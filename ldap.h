/**
 *
 * @file ldap.h
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  Last modified: Oct 26, 2023
 *
 *
 */

#ifndef _LDAP_H
#define _LDAP_H

void ldap(int clientSocket, FILE *file);

/**
 * Receive LDAP data from a client socket.
 *
 * This function receives LDAP data from the specified client socket
 * and stores it in a buffer. It also updates the receivedBytes variable
 * with the number of bytes received.
 *
 * @param clientSocket The socket to receive data from.
 * @param receivedBytes A pointer to a size_t variable to store the number of received bytes.
 *
 * @return A pointer to the received data (a dynamically allocated buffer).
 *         The caller is responsible for freeing this buffer.
 */
unsigned char *ldap_receive(int clientSocket, size_t *receivedBytes);
/**
 * Parse an LDAP request to determine the LDAP operation.
 *
 * This function analyzes the provided LDAP request data to determine the type
 * of LDAP operation it represents, such as bind, search, modify, etc.
 *
 * @param data The LDAP request data to be parsed.
 * @param length The length of the LDAP request data.
 *
 * @return 0 if the LDAP request is successfully parsed and represents a valid LDAP operation.
 *         A non-zero value is returned if an error occurs during parsing.
 */
int ldap_handle_request(unsigned char *data, size_t length, int clientSocket, FILE *file);

void add_ldap_oid(unsigned char *buff, int *offset, char *string);
void ldap_notice_of_disconnection(int clientSocket);
#endif