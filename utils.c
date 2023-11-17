/**
 *
 * @file utils.c
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "utils.h"

extern int currentTagPosition;

void create_ldap_header(unsigned char *buff, int *offset, int messageId)
{
    add_ldap_byte(buff, offset, LDAP_MESSAGE_PREFIX);
    add_ldap_byte(buff, offset, LDAP_PLACEHOLDER); // placeholder value

    add_integer(buff, offset, messageId);
}
void add_integer(unsigned char *buff, int *offset, int value)
{
    int numBytes = 0;
    int temp = value;

    while (temp > 0)
    {
        temp >>= 8; // Shift 'temp' right by 8 bits
        numBytes++;
    }

    add_ldap_byte(buff, offset, INTEGER_TYPE);
    if (value == 0)
    {
        add_ldap_byte(buff, offset, 1);
        add_ldap_byte(buff, offset, 0);
    }
    else
    {
        add_ldap_byte(buff, offset, numBytes);
        // Add the integer data bytes (in big-endian byte order)
        for (int i = numBytes; i > 0; i--)
        {
            add_ldap_byte(buff, offset, (value >> ((i - 1) * 8)) & 0xFF); // 0xff mask to show only the lowest byte
        }
    }
}

void add_ldap_byte(unsigned char *buff, int *offset, int value)
{
    buff[*offset] = (unsigned char)value;
    (*offset)++;
}

void add_ldap_string(unsigned char *buff, int *offset, char *string)
{
    int lenght = strlen(string);
    add_ldap_byte(buff, offset, OCTET_STRING_TYPE);
    add_ldap_byte(buff, offset, lenght);
    for (size_t i = 0; i < lenght; i++)
    {
        add_ldap_byte(buff, offset, string[i]);
    }
}

void add_ldap_oid(unsigned char *buff, int *offset, char *string)
{
    int lenght = strlen(string);
    add_ldap_byte(buff, offset, EXTENDED_RESPONSE_OID);
    add_ldap_byte(buff, offset, lenght);
    for (size_t i = 0; i < lenght; i++)
    {
        add_ldap_byte(buff, offset, string[i]);
    }
}

LdapElementInfo get_ldap_element_info(unsigned char *data)
{
    LdapElementInfo elementInfo;
    elementInfo.tagValue = data[currentTagPosition];

    int lengthValue = data[currentTagPosition + 1];
    if (elementInfo.tagValue <= POSTFIX && elementInfo.tagValue >= PREFIX)
    {
        set_universal_type(data, &elementInfo, lengthValue);
    }
    else if (elementInfo.tagValue > 0x3F || elementInfo.tagValue == LDAP_MESSAGE_PREFIX) // universal type
    {
        set_application_type(data, &elementInfo, lengthValue);
    }
    else
    {
        set_universal_type(data, &elementInfo, lengthValue);
    }
    print_ldap_element_info(elementInfo);
    currentTagPosition = elementInfo.nextTagPosition;
    return elementInfo;
}

void set_application_type(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue)
{

    if (lengthValue >= 0x81 && lengthValue <= 0xFE)
        elementInfo->lengthOfData = lengthValue - 0x80;

    if (lengthValue <= 0x76)
        elementInfo->lengthOfData = 0;

    elementInfo->start = currentTagPosition + 1;
    elementInfo->end = elementInfo->start;
    elementInfo->nextTagPosition = currentTagPosition + elementInfo->lengthOfData + 2;
}

void set_universal_type(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue)
{

    if (lengthValue <= 0x76) // maximum of length that can be encoded in one byte
    {

        elementInfo->lengthOfData = data[currentTagPosition + 1];
        elementInfo->start = currentTagPosition + 2;
        elementInfo->end = elementInfo->start + elementInfo->lengthOfData - 1;
        elementInfo->nextTagPosition = currentTagPosition + elementInfo->lengthOfData + 2;
    }
    if (lengthValue >= 0x81 && lengthValue <= 0xFE)
    {
        get_long_length_info(data, elementInfo, lengthValue);
        elementInfo->end = elementInfo->start + elementInfo->lengthOfData - 1;
        elementInfo->nextTagPosition = currentTagPosition + elementInfo->lengthOfData + lengthValue - 0x80 + 2;
    }
}

void get_long_length_info(unsigned char *data, LdapElementInfo *elementInfo, int lengthValue)
{

    int lengthOfLength = lengthValue - 0x80;
    elementInfo->start = currentTagPosition + 1 + lengthOfLength;
    elementInfo->lengthOfData = 0;
    for (int i = currentTagPosition + 2; i <= elementInfo->start; i++)
    {
        elementInfo->lengthOfData = elementInfo->lengthOfData * 256 + (int)data[i];
    }
}

void print_ldap_element_info(LdapElementInfo elementInfo)
{
    debug(3, "Tag Value: %02X\n", elementInfo.tagValue);
    debug(3, "Length of Data: %d\n", elementInfo.lengthOfData);
    debug(3, "Start: %d\n", elementInfo.start);
    debug(3, "End: %d\n", elementInfo.end);
    debug(3, "New Tag Position: %d\n\n", elementInfo.nextTagPosition);
}

void debug(int level, const char *format, ...)
{
    if (DEBUG_LEVEL >= level)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

char *get_string_value(unsigned char *data)
{
    LdapElementInfo LdapInfo = get_ldap_element_info(data);

    char *extractedString = (char *)malloc(LdapInfo.lengthOfData + 1); // +1 for null terminator
    if (extractedString)
    {
        for (int i = LdapInfo.start, j = 0; i <= LdapInfo.end; i++, j++)
        {
            extractedString[j] = data[i];
        }
        extractedString[LdapInfo.lengthOfData] = '\0'; // Null-terminate the string

        return extractedString;
    }
    else
    {
        printf("Allocation failed.");
        return NULL; // Return an error indicator
    }
}

long long get_int_value(unsigned char *data)
{

    LdapElementInfo LdapInfo = get_ldap_element_info(data);

    // Extract the integer value
    long long combinedDecimalNumber = 0;

    if (LdapInfo.end != LdapInfo.start)
    {
        for (int i = LdapInfo.start; i <= LdapInfo.end; i++)
        {
            combinedDecimalNumber = combinedDecimalNumber * 256 + data[i];
        }
    }
    else
    {
        combinedDecimalNumber = data[LdapInfo.start];
    }

    return combinedDecimalNumber;
}

void print_hex_message(const unsigned char *data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        debug(2, "%02X ", data[i]); // Print each byte in hexadecimal format
    }
    debug(2, "\n\n");
}