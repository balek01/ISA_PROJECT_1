
#ifndef _UTILS_H
#define _UTILS_H

enum MyConst
{
    LDAP_MESSAGE_PREFIX = 0x30,
    MAX_BUFFER_SIZE = 2048,
    LDAP_MSG_LENGTH_OFFSET = 1,
    LDAP_PLACEHOLDER = 0x00
};

enum SubstringType
{
    PREFIX = 0x80,
    INFIX = 0x81,
    POSTFIX = 0x82
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
    ENUMERATED_TYPE = 0x0A
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

void get_long_length_info(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue);
LdapElementInfo get_ldap_element_info(unsigned char *data);
void add_integer(unsigned char *buff, int *offset, int value);
void set_application_type(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue);
void set_universal_type(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue);
void add_ldap_string(unsigned char *buff, int *offset, char *string);
void ldap_search_res_done(unsigned char *buff, int *offset, int returnCode, int clientSocket);
#endif