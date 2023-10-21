/**
 *
 * @file ldap.h
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *  Last modified: Oct 21, 2023
 *
 *
 */

#ifndef _LDAP_H
#define _LDAP_H

int LDAP_MESSAGE_PREFIX = 0x30;
int MAX_BUFFER_SIZE = 2048;
int LDAP_MSG_LENGTH_OFFSET = 1;

enum CSVOffset
{
    COMMON_NAME = 0,
    UID = 1,
    MAIL = 2,
};

enum LDAPPrtotocolOp
{
    LDAP_BIND_REQUEST = 0x60,         // 96
    LDAP_BIND_RESPONSE = 0x61,        // 97
    LDAP_UNBIND_REQUEST = 0x42,       // 66
    LDAP_SEARCH_REQUEST = 0x63,       // 99
    LDAP_SEARCH_RESPONSE = 0x64,      // 100
    LDAP_MODIFY_REQUEST = 0x66,       // 102
    LDAP_MODIFY_RESPONSE = 0x67,      // 103
    LDAP_ADD_REQUEST = 0x68,          // 104
    LDAP_ADD_RESPONSE = 0x69,         // 105
    LDAP_DELETE_REQUEST = 0x4A,       // 74
    LDAP_DELETE_RESPONSE = 0x6B,      // 107
    LDAP_MODIFY_DN_REQUEST = 0x6C,    // 108
    LDAP_MODIFY_DN_RESPONSE = 0x6D,   // 109
    LDAP_COMPARE_REQUEST = 0x6E,      // 110
    LDAP_COMPARE_RESPONSE = 0x6F,     // 111
    LDAP_ABANDON_REQUEST = 0x50,      // 80
    LDAP_SEARCH_RESULT_ENTRY = 0x64,  // 100
    LDAP_SEARCH_RESULT_DONE = 0x65,   // 101
    LDAP_EXTENDED_REQUEST = 0x77,     // 119
    LDAP_EXTENDED_RESPONSE = 0x78,    // 120
    LDAP_INTERMEDIATE_RESPONSE = 0x79 // 121
};

enum TagType
{
    EOC_TYPE = 0x00,
    BOOLEAN_TYPE = 0x01,
    INTEGER_TYPE = 0x02,
    BIT_STRING_TYPE = 0x03,
    OCTET_STRING_TYPE = 0x04,
    NULL_TYPE = 0x05,
    ENUMERATED_TYPE = 0x0A
};

enum ResultCode
{
    SUCCESS = 0,
    OPPERATION_ERROR = 1,
    PROTOCOL_ERROR = 2,
    TIME_LIMIT_EXCEEDED = 3,
    SIZE_LIMIT_EXCEEDED = 4,
    UNSUPORTED_FILTER = 5,
    UNWILLING_TO_PERFORM = 53
};

enum FilterType
{
    AND_FILTER = 0xA0,
    OR_FILTER = 0xA1,
    NOT_FILTER = 0xA2,
    EQUALITY_MATCH_FILTER = 0xA3,
    SUBSTRING_FILTER = 0xA4
};

enum SubstringType
{
    PREFIX = 0x80,
    INFIX = 0x81,
    POSTFIX = 0x82
};

/**
 * Structure representing an LDAP Bind message.
 *
 * The LdapBind structure is used to represent an LDAP Bind message, containing
 * essential information about the message, including the message ID and the protocol version.
 */
typedef struct
{
    int messageId; /**< The unique identifier for the LDAP Bind message. */
    int version;   /**< The protocol version associated with the LDAP Bind message. */
} LdapBind;

/**
 * Structure representing an LDAP Filter for search criteria.
 *
 * The LdapFilter structure is used to represent an LDAP Filter.
 * It includes the attribute description, attribute value, and filter type.
 */
typedef struct
{
    char *attributeDescription; /**< The description of the attribute being filtered. */
    char *attributeValue;       /**< The value used for the filter. */
    enum FilterType filterType; /**< The type of filter (e.g., equality, presence, etc.). */
    enum SubstringType substringType;
} LdapFilter;

/**
 * Structure representing an LDAP Search request.
 *
 * The LdapSearch structure is used to represent an LDAP Search request, containing
 * information about the search criteria, such as the base object, scope, and filter.
 */
typedef struct
{
    int messageId;              /**< The unique identifier for the LDAP  message. */
    char *baseObject;           /**< The base object for the LDAP search. */
    int scope;                  /**< The search scope (base, one-level, or subtree). */
    int derefAliases;           /**< How alias dereferencing should be handled. */
    int sizeLimit;              /**< Maximum number of entries to return. */
    int timeLimit;              /**< Maximum time allowed for the search. */
    int typesOnly;              /**< Flag indicating whether to return attribute types only (0 for no, 1 for yes). */
    LdapFilter filter;          /**< The LDAP filter for the search. */
    enum ResultCode returnCode; /**< Return code indicating whether the search was successful. */
} LdapSearch;

/**
 * Structure representing the result of an LDAP operation.
 *
 * The LDAPResult structure is used to store the result of an LDAP operation,
 * including a result code, matched distinguished name (DN), and diagnostic message.
 */
typedef struct
{
    uint8_t resultCode;          /**< The result code indicating the outcome of the LDAP operation. */
    char matchedDN[128];         /**< The distinguished name (DN) that matched the LDAP operation. Adjust size as needed. */
    char diagnosticMessage[128]; /**< A diagnostic message providing additional information. Adjust size as needed. */
} LDAPResult;

/**
 * Structure representing the response to an LDAP Bind operation.
 *
 * The BindResponse structure is used to represent the response to an LDAP Bind operation,
 * including an LDAP result and an optional field for server SASL credentials.
 */
typedef struct
{
    LDAPResult ldapResult;     /**< The result of the LDAP Bind operation. */
    char serverSaslCreds[128]; /**< Optional server SASL credentials. Adjust size as needed. */
} BindResponse;

/**
 * Structure representing LDAP element information.
 *
 * The LdapElementInfo structure is used to store detailed information about an LDAP element.
 * It contains various properties related to the element's position, data length, and tag value.
 */
typedef struct
{
    int start;              /**< The start position of value in the LDAP element. */
    int end;                /**< The end position of value in the LDAP element. */
    int lengthOfData;       /**< The length of value in the LDAP element. */
    int nextTagPosition;    /**< The position of the next LDAP tag in the data. */
    unsigned char tagValue; /**< The tag value associated with the LDAP element. */
} LdapElementInfo;

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
int ldap_parse_request(unsigned char *data, size_t length, int clientSocket, FILE *file);

LdapBind ldap_bind(unsigned char *data, int message_id);

/**
 * Print a hexadecimal representation of received data.
 *
 * This function prints the hexadecimal representation of received data to the standard
 * output. It is useful for displaying received data in a hexadecimal format.
 *
 * @param data   A pointer to the received data to be printed in hexadecimal.
 * @param length The length of the received data in bytes.
 */
void print_hex_message(const unsigned char *data, size_t length);

/**
 * Print LDAP element information to the standard output.
 *
 * This function is used to display detailed information about an LDAP element,
 * including its start position, end position, length of data, next tag position,
 * and tag value, to the standard output.
 *
 */
void print_ldap_element_info(LdapElementInfo elementInfo);

/**
 * Extract an integer value from received data at the element position indicated by 'currentTagPosition'.
 *
 * This function retrieves an integer value from the received data at the element position specified
 * by the global variable 'currentTagPosition' and returns it as a long long integer.
 *
 * @param data A pointer to the data containing the integer value.
 * @return The extracted integer value as a long long integer.
 */
long long get_int_value(unsigned char *data);

/**
 * Extract a string value from received data at the element position indicated by 'currentTagPosition'.
 *
 * This function retrieves a string value from the received data at the element position specified
 * by the global variable 'currentTagPosition' and increments the 'currentTagPosition'.
 *
 * @param data A pointer to the data containing the string value.
 * @return The extracted string value or NULL if allocation failed.
 */
char *get_string_value(unsigned char *data);

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

/**
 * Add a byte to an LDAP message buffer and update the offset.
 *
 * This function adds a byte with the specified value to an LDAP message buffer,
 * and it also updates the offset to the next position in the buffer.
 *
 * @param buff A pointer to the LDAP message buffer.
 * @param offset A pointer to the offset indicating the current position in the buffer.
 * @param value The byte value to be added to the buffer.
 */
void add_ldap_byte(unsigned char *buff, int *offset, int value);

/**
 * Create an LDAP message header and add it to a message buffer.
 *
 * This function creates an LDAP message header for a message with the specified
 * message ID and adds it to the LDAP message buffer at the current offset.
 *
 * @param buff A pointer to the LDAP message buffer.
 * @param offset A pointer to the offset indicating the current position in the buffer.
 * @param messageId The unique identifier for the LDAP message.
 */
void create_ldap_header(unsigned char *buff, int *offset, int messageId);

void get_long_length_info(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue);
LdapElementInfo get_ldap_element_info(unsigned char *data);
void ldap_send(unsigned char *bufin, int clientSocket, int offset);
LdapSearch ldap_search(unsigned char *data, int messageId);
LdapFilter get_ldap_filter(unsigned char *data, LdapSearch *search);
void dispose_ldap_search(LdapSearch search);
void ldap_search_response(LdapSearch search, int clientSocket, FILE *file);
void print_ldap_search(LdapSearch search);
void ldap_search_res_done(unsigned char *buff, int *offset, int returnCode, int clientSocket);
void add_ldap_string(unsigned char *buff, int *offset, char *string);
int get_targeted_column(LdapFilter filter);
void add_integer(unsigned char *buff, int *offset, int value);
void ldap_add_search_matches(unsigned char *buff, int *offset, LdapFilter filter, FILE *file);

#endif