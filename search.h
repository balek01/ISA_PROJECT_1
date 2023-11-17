/**
 *
 * @file search.h
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *
 *
 */

#ifndef _SEARCH_H
#define _SEARCH_H

typedef struct
{
    char *uid;
    char *cn;
    char *mail;
} FileLine;

enum FilterType
{
    AND_FILTER = 0xA0,
    OR_FILTER = 0xA1,
    NOT_FILTER = 0xA2,
    EQUALITY_MATCH_FILTER = 0xA3,
    SUBSTRING_FILTER = 0xA4
};

enum CSVOffset
{
    COMMON_NAME = 0,
    UID = 1,
    MAIL = 2,
};

enum LdapSearchResponseCodes
{
    LDAP_PARTIAL_ATTRIBUTE_LIST = 0x30,
    LDAP_PARTIAL_ATTRIBUTE_LIST_VALUE = 0x31,
};

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
    char *attributeValue2;      /**< The second value used for the filter. */
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
 * LDAP Search Operation.
 *
 * Initiates an LDAP search operation using the provided data and message ID.
 *
 * @param data A pointer to the data required for the search operation.
 * @param messageId An integer representing the unique identifier for the LDAP message.
 *
 * @return An LdapSearch structure representing the result of the search operation.
 */
LdapSearch ldap_search(unsigned char *data, int messageId);

/**
 * Get LDAP Filter.
 *
 * Extracts an LDAP filter from the provided data and associates it with the given LDAP search.
 *
 * @param data      A pointer to the data containing information for the LDAP filter.
 * @param search    A pointer to the LdapSearch structure to associate the filter with.
 *
 * @return          An LdapFilter structure representing the extracted LDAP filter.
 *                  The caller is responsible for managing any associated memory.
 */
LdapFilter get_ldap_filter(unsigned char *data, LdapSearch *search);

/**
 * Dispose LDAP Search.
 *
 * Releases resources associated with the provided LdapSearch structure.
 *
 * @param search    The LdapSearch structure to be disposed of.
 *             
 */
void dispose_ldap_search(LdapSearch search);

/**
 * LDAP Search Response.
 *
 * Sends an LDAP search response.
 *
 * @param search        The LdapSearch structure containing information for the search response.
 * @param clientSocket  The socket to which the LDAP search response will be sent.
 * @param file          A pointer to a FILE structure representing the database file.
 */
void ldap_search_response(LdapSearch search, int clientSocket, FILE *file);

/**
 * Print LDAP Search.
 *
 * Prints the content of the provided LdapSearch structure to the standard output.
 */
void print_ldap_search(LdapSearch search);

/**
 * Check Token Equality with LDAP Filter Value.
 *
 * Compares the provided token to the value in the given LDAP filter.
 *
 * @param filter    The LdapFilter structure containing the filter value to compare.
 * @param token     A pointer to the token to compare with the filter value.
 *
 * @return          Returns true if the token is equal to the filter value, false otherwise.
 */
bool is_token_equal_filter_value(LdapFilter filter, char *token);

/**
 * LDAP Send Search Result Entry.
 *
 * Creates and sends an LDAP search result entry based on the provided FileLine structure.
 *
 * @param buff          A pointer to the buffer where the LDAP search result entry will be constructed.
 * @param offset        A pointer to the offset in the buffer where the LDAP search result entry will be added.
 * @param fl            The FileLine structure containing information for constructing the LDAP entry.
 * @param clientSocket  The socket to which the LDAP search result entry will be sent.
 */
void ldap_send_search_res_entry(unsigned char *buff, int *offset, FileLine fl, int clientSocket);

/**
 * LDAP Send Search Result Entries.
 *
 * Retrieves entries from a file based on the LDAP search filter, constructs
 * LDAP search result entries, and sends them to the specified client socket.
 *
 * @param buff          A pointer to the buffer where the LDAP search result entries will be constructed.
 * @param offset        A pointer to the offset in the buffer where the LDAP search result entries will be added.
 * @param search        A pointer to the LdapSearch structure containing search parameters.
 * @param file          A pointer to the FILE structure representing the database file.
 * @param clientSocket  The socket to which the LDAP search result entries will be sent.
 */
void ldap_send_search_res_entrys(unsigned char *buff, int *offset, LdapSearch *search, FILE *file, int clientSocket);

/**
 * Add LDAP Attribute list to LDAP response.
 *
 * Adds an LDAP attribute with the specified type and value to the buffer
 * at the specified offset.
 *
 * @param buff      A pointer to the buffer where the LDAP attribute will be added.
 * @param offset    A pointer to the offset in the buffer where the LDAP attribute will be added.
 * @param type      A pointer to the string representing the attribute type.
 * @param value     A pointer to the string representing the attribute value.
 */
void add_ldap_attribute_list(unsigned char *buff, int *offset, char *type, char *value);

/**
 * Remove EOL characters from string.
 *
 * Modifies the provided string in-place by removing EOL characters
 *
 * @param str   A pointer to the  string to be modified.
 */
void removeEOL(char *str);

/**
 * Get Targeted Column based on LDAP Filter Attribute.
 *
 * Converts the attribute description in the provided LDAP filter to lowercase
 * and determines the corresponding column based on recognized attribute names.
 *
 * @param filter    The LdapFilter structure containing the attribute description.
 *
 * @return          An integer representing the targeted column based on the attribute.
 *                  Returns -1 if the attribute is not recognized.
 */
int get_targeted_column(LdapFilter filter);

/**
 * Convert String to Lowercase.
 *
 * Converts all characters in the provided string to lowercase.
 *
 * @param string  A pointer to the string to be converted to lowercase.
 */
void to_lowercase(char *string);
void ldap_send(unsigned char *bufin, int clientSocket, int offset);

#endif