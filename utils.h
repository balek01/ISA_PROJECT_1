/**
 *
 * @file utils.h
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *
 */
#ifndef _UTILS_H
#define _UTILS_H

enum MyConst
{
    LDAP_MESSAGE_PREFIX = 0x30,
    MAX_BUFFER_SIZE = 2048,
    LDAP_MSG_LENGTH_OFFSET = 1,
    LDAP_PLACEHOLDER = 0x00,
    DEBUG_LEVEL = 0 // change this in range <0,3> 
};

enum SubstringType
{
    PREFIX = 0x80,
    INFIX = 0x81,
    POSTFIX = 0x82,
    ANY_CENTER = 0x83 // made up value for abc*def filters
};

enum LDAPPrtotocolOp
{
    LDAP_BIND_REQUEST = 0x60,          // 96
    LDAP_BIND_RESPONSE = 0x61,         // 97
    LDAP_UNBIND_REQUEST = 0x42,        // 66
    LDAP_SEARCH_REQUEST = 0x63,        // 99
    LDAP_SEARCH_RESPONSE = 0x64,       // 100
    LDAP_MODIFY_REQUEST = 0x66,        // 102
    LDAP_MODIFY_RESPONSE = 0x67,       // 103
    LDAP_ADD_REQUEST = 0x68,           // 104
    LDAP_ADD_RESPONSE = 0x69,          // 105
    LDAP_DELETE_REQUEST = 0x4A,        // 74
    LDAP_DELETE_RESPONSE = 0x6B,       // 107
    LDAP_MODIFY_DN_REQUEST = 0x6C,     // 108
    LDAP_MODIFY_DN_RESPONSE = 0x6D,    // 109
    LDAP_COMPARE_REQUEST = 0x6E,       // 110
    LDAP_COMPARE_RESPONSE = 0x6F,      // 111
    LDAP_ABANDON_REQUEST = 0x50,       // 80
    LDAP_SEARCH_RESULT_ENTRY = 0x64,   // 100
    LDAP_SEARCH_RESULT_DONE = 0x65,    // 101
    LDAP_EXTENDED_REQUEST = 0x77,      // 119
    LDAP_EXTENDED_RESPONSE = 0x78,     // 120
    LDAP_INTERMEDIATE_RESPONSE = 0x79, // 121

};
enum ResultCode
{
    SUCCESS = 0,
    OPPERATION_ERROR = 1,
    PROTOCOL_ERROR = 2,
    TIME_LIMIT_EXCEEDED = 3,
    SIZE_LIMIT_EXCEEDED = 4,
    UNSUPORTED_FILTER = 5,
    AUTH_METHOD_NOT_SUPPORTED = 7,
    INVALID_DN_SYNTAX = 34,
    UNAVAILABLE = 52,
    UNWILLING_TO_PERFORM = 53
};
enum TagType
{
    EOC_TYPE = 0x00,
    BOOLEAN_TYPE = 0x01,
    INTEGER_TYPE = 0x02,
    BIT_STRING_TYPE = 0x03,
    OCTET_STRING_TYPE = 0x04,
    NULL_TYPE = 0x05,
    ENUMERATED_TYPE = 0x0A,
    EXTENDED_RESPONSE_OID = 0x8A
};

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

/**
 * Get Information for Long Length Encoding.
 *
 * Extracts information about the length of data encoded using long length format
 * and updates the provided LdapElementInfo structure.
 *
 * @param data          A pointer to the data containing the LDAP element.
 * @param elementInfo   A pointer to the LdapElementInfo structure to be updated.
 * @param lengthValue   The length value indicating long length encoding.
 */
void get_long_length_info(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue);

/**
 * Get LDAP Element Information.
 *
 * Extracts information about an LDAP element from the provided data.
 *
 * @param data              A pointer to the data containing information for the LDAP element.
 *
 * @return                  An LdapElementInfo structure representing information about the LDAP element.
 *                          The caller is responsible associated memory.
 */
LdapElementInfo get_ldap_element_info(unsigned char *data);

/**
 * Add Integer to Response.
 *
 * Adds the provided integer value to the buffer at the specified offset.
 *
 * @param buff      A pointer to the buffer where the integer will be added.
 * @param offset    A pointer to the offset in the buffer where the integer will be added.
 * @param value     The integer value to add to the buffer.
 */
void add_integer(unsigned char *buff, int *offset, int value);

/**
 * Set Application Type Information in LDAP Element.
 *
 * Analyzes the length value of an LDAP element and updates the provided
 * LdapElementInfo structure.
 *
 * @param data          A pointer to the data containing the LDAP element.
 * @param elementInfo   A pointer to the LdapElementInfo structure to be updated.
 * @param lengthValue   The length value of the LDAP element.
 */
void set_application_type(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue);

/**
 * Set Universal Type Information in LDAP Element.
 *
 * Analyzes the length value of an LDAP element and updates the provided
 * LdapElementInfo.
 *
 * @param data          A pointer to the data containing the LDAP element.
 * @param elementInfo   A pointer to the LdapElementInfo structure to be updated.
 * @param lengthValue   The length value of the LDAP element.
 */
void set_universal_type(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue);

/**
 * Add LDAP String to Buffer.
 *
 * Adds the provided LDAP string to the buffer at the specified offset.
 *
 * @param buff      A pointer to the buffer where the LDAP string will be added.
 * @param offset    A pointer to the offset in the buffer where the LDAP string will be added.
 * @param string    A pointer to the LDAP string to add to the buffer.
 */
void add_ldap_string(unsigned char *buff, int *offset, char *string);

/**
 * Add LDAP Object Identifier to Buffer.
 *
 * Adds the provided LDAP Object Identifier string to the buffer at the specified offset.
 *
 * @param buff      A pointer to the buffer where the LDAP OID will be added.
 * @param offset    A pointer to the offset in the buffer where the LDAP OID will be added.
 * @param string    A pointer to the LDAP OID string to add to the buffer.
 *                  The string is expected to be a valid OID representation.
 */
void add_ldap_oid(unsigned char *buff, int *offset, char *string);

/**
 * LDAP Search Result Done.
 *
 * Adds LDAP search result done information to the buffer at the specified offset
 * and sends it to the specified client socket.
 *
 * @param buff          A pointer to the buffer where the LDAP search result done information will be added.
 * @param offset        A pointer to the offset in the buffer where the information will be added.
 * @param returnCode    An integer representing the return code for the LDAP search result done.
 * @param clientSocket  The socket to which the LDAP search result done information will be sent.
 */
void ldap_search_res_done(unsigned char *buff, int *offset, int returnCode, int clientSocket);

/**
 * Debugging Output.
 *
 * Outputs debugging information based on the specified level and format string,
 * similar to the behavior of printf. Variable arguments can be used to include
 * additional values in the output.
 *
 * @param level     The level of the debugging information.
 * @param format    The format string for the debugging output.
 * @param ...       Additional variable arguments to be included in the output.
 */
void debug(int level, const char *format, ...);
#endif