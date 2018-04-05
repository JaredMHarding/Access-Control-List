#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>

#define MAXNAMESIZE 256

typedef struct _nameAccessPair {
    char* name;
    char access;
    struct _nameAccessPair* next;
} nap;

nap* newNAP() {
    nap* n = malloc(sizeof(nap));
    n->name = calloc(MAXNAMESIZE,sizeof(char));
    n->access = '\0';
    n->next = NULL;
    return n;
}

void freeNAP(nap* head) {
    nap* cursor = head;
    while (cursor != NULL) {
        free(cursor->name);
        nap* next = cursor->next;
        free(cursor);
        cursor = next;
    }
}

int validateACL(char* acl, nap* head) {
    if (head == NULL) {
        fprintf(stderr,"nap pointer was NULL\n");
        return -1;
    }
    nap* cursor = head;
    FILE* aclFILE = fopen(acl,"r");
    if (aclFILE == NULL) {
        perror("fopen");
        return -1;
    }
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line,&len,aclFILE)) != -1) {
        if (line[0] == '#') {
            continue;
        }
        // this is the index for the line made from getline()
        int i = 0;
        // this is the cursor index for cursor->name
        int ci = 0;
        // gets to first non whitespace char
        while ((size_t)i < len) {
            char ch = line[i++];
            if (ch == '\n') {
                // newline happened too soon
                return -1;
            }
            else if (isspace(ch)) {
                continue;
            }
            else {
                // it was an actual character, so add it to cursor->name
                if (ci < MAXNAMESIZE) {
                    cursor->name[ci++] = ch;
                }
                // no room
                else {
                    fprintf(stderr,"char* name in nap*: Array is full\n");
                    return -1;
                }
                break;
            }
        }
        // start of the uid
        while ((size_t)i < len) {
            char ch = line[i++];
            if (ch == '\n') {
                // new line happened too soon
                return -1;
            }
            else if (isspace(ch)) {
                // end of uid
                break;
            }
            else {
                // it was an actual character, so add it to cursor->name
                if (ci < MAXNAMESIZE) {
                    cursor->name[ci++] = ch;
                }
                // no room
                else {
                    fprintf(stderr,"char* name in nap*: Array is full\n");
                    return -1;
                }
            }
        }
        // makes sure there is a second char after the uid
        // either r, w, or b
        while ((size_t)i < len) {
            char ch = line[i++];
            if (ch == '\n') {
                // still too soon for a newline
                return -1;
            }
            else if (isspace(ch)) {
                continue;
            }
            // these characters are good
            else if ((ch == 'r') || (ch == 'w') || (ch == 'b')) {
                cursor->access = ch;
                break;
            }
            // any other character at this point is invalid
            else {
                return -1;
            }
        }
        // only white space characters past this point
        while ((size_t)i < len) {
            char ch = line[i++];
            if (ch == '\n') {
                // newline at this point is ok
                break;
            }
            // white space is fine
            else if (isspace(ch)) {
                continue;
            }
            // any other character at this point is invalid
            else {
                return -1;
            }
        }
        // line was good, set up cursor for next line
        nap* nextNAP = newNAP();
        cursor->next = nextNAP;
        cursor = nextNAP;
    }
    free(line);
    fclose(aclFILE);
    // all of the lines were valid
    return 0;
}