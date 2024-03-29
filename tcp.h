/**
 *
 * @file tcp.h
 *
 * @brief Project: ISA LDAP server
 *
 * @author xbalek02 Miroslav Bálek
 *
 *
 *
 *
 */

#ifndef _TCP_H
#define _TCP_H

/**
 * Maximum of users that can be connected to the server
 */
int MAX_USERS = 500;
int DEFAULT_PORT = 389;

/**
 * @struct Conn
 * @brief Represents a connection configuration.
 *
 * The `Conn` struct holds information about a connection, including the `port` number
 * and the `ldap database file`.
 *
 * @var int Conn::port
 * The port number at which the serrver is running
 *
 * @var char* Conn::file
 * Path to the csv file containing ldap database
 */
typedef struct
{
    int port;
    char *filePath;
    FILE *filePtr;

} Conn;

/**
 * Parse given arguments int Conn struct.
 *
 * @param argc Count of arguments passed by user.
 * @param argv Array od arguments.
 *
 * @return Struct Conn containing connection information.
 */
Conn ParseArgs(int argc, char *const argv[]);

/**
 * Create TCP IPv6 socket
 *
 *  @return File descriptor of created socket.
 *
 */
int CreateSocket();

/**
 * Bind any address on the socket with specified port
 *  @param conn Conn structure containig connection information
 *
 */
void BindSocket(Conn conn);

/**
 * Listen for incoming connections
 *
 *  @param conn Conn structure containig connection information
 *
 */
void Listen(Conn conn);

/**
 * Accept connection from client
 * Creates new process using fork and new client socket
 *  @param conn Conn structure containig connection information
 *
 */
void Accept(Conn conn);

#endif