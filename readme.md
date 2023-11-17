# ISA_PROJECT_1(LDAP SERVER)


## Brief
The project implements a simplified server for the LDAP protocol. The program establishes the connection and communicates with the cient in the way specified for this protocol. Server is running at specified port listening to all ip addresses on both IPv4 and IPv6. Client then sends ldap search and the server responds with ldap response, containing requested information. Server searches simple semicolon separated csv for requested information.

## Known Limitations 
Server supports only equality match filters and substring filters. If a substring filter with multiple * is used the server behaves as if it received a prefix filter and ignores the rest of upcoming filters. 

## Example of usage 
```
./isa-ldapserver -f lidi.csv -p 12345
```

## Submitted files
```
├── bind.c
├── bind.h
├── ldap.c
├── ldap.h
├── Makefile
├── manual.md
├── manual.pdf
├── readme.md
├── search.c
├── search.h
├── tcp.c
├── tcp.h
├── test.py
├── utils.c
└── utils.h
```

## Author

- [Miroslav Bálek](https://github.com/balek01) (xbalek02)
- Created on 17.11.2023


