# ISA_PROJECT_1

## Content table
1. [ Brief ](#brief)
2. [ TCP ](#tcp)
3. [ TCP-Server ](#tcp-server)

## Brief
The project implements a simplified server for the LDAP protocol. The program establishes the connection and communicates with the cient in the way specified for this protocol. Server is running at specified port listenig to all ip addresses on both IPv4 and IPv6. Client then sends ldap search and the server responds with ldap response, containing requested information. Server searches simple semicolon separated csv for requested information.

## TCP 
TCP (Transmission Control Protocol) is a reliable, connection-oriented communication protocol used in computer networks. It provides ordered and authenticated data delivery between devices. TCP establishes a connection using a three-way handshake, where it sets parameters for communication. It provides flow control to prevent data congestion and uses delivery acknowledgement mechanisms to confirm the successful transmission of data. TCP is used for applications that require secure and accurate data transfer, such as browsers and file transfer.

## TCP-Server
TCP communication first creates a socket. Then created socket is binded to specific ip address and port. In this case the socket is binded to in6addr_any (::) and port specified at launch of server (default port is 389). Server then listens on this socket for any incoming connections. When connection request is recieved, server accepts this connection at new socket and creates new  process for this connection.  After comunication is done the server closes this socket.

## Ldap-Server

## Author
- [Miroslav Bálek](https://github.com/balek01)