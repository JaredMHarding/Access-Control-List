#ifndef VALIDACL
#define VALIDACL

#define MAXNAMESIZE 256

typedef struct _nameAccessPair {
    char* name;
    char access;
    struct _nameAccessPair* next;
} nap;

nap* newNAP();

void freeNAP(nap* head);

int validateACL(char* acl, nap* head);

#endif // VALIDACL