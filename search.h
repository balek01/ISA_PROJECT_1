/**
 *
 * @file search.h
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

LdapSearch ldap_search(unsigned char *data, int messageId);
LdapFilter get_ldap_filter(unsigned char *data, LdapSearch *search);
void dispose_ldap_search(LdapSearch search);
void ldap_search_response(LdapSearch search, int clientSocket, FILE *file);
void print_ldap_search(LdapSearch search);
bool is_token_equal_filter_value(LdapFilter filter, char *token);
void ldap_send_search_res_entry(unsigned char *buff, int *offset, FileLine fl, int clientSocket);
void ldap_send_search_res_entrys(unsigned char *buff, int *offset, LdapSearch *search, FILE *file, int clientSocket);
void add_ldap_attribute_list(unsigned char *buff, int *offset, char *type, char *value);
void removeEOL(char *str);
int get_targeted_column(LdapFilter filter);
void to_lowercase(char *string);
void ldap_send(unsigned char *bufin, int clientSocket, int offset);

#endif