#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "validACL.h"

#define MAXPATHSIZE 4096
#define BUFSIZE 8192

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr,"Usage: ./get source destination\n");
        exit(1);
    }
    if (strlen(argv[1]) >= MAXPATHSIZE) {
        fprintf(stderr,"Source pathname '%s' too long\n",argv[1]);
        exit(1);
    }
    if (strlen(argv[2]) >= MAXPATHSIZE) {
        fprintf(stderr,"Destination pathname '%s' too long\n",argv[2]);
        exit(1);
    }
    uid_t uid = getuid(); // user running the get program
    uid_t euid = geteuid(); // user who owns the src file
    char src[MAXPATHSIZE]; // euid's file to be copied
    char dest[MAXPATHSIZE]; // uid's location to place the copy
    strncpy(src,argv[1],MAXPATHSIZE);
    strncpy(dest,argv[2],MAXPATHSIZE);
    // make the string for the <src>.access file
    char acl[MAXPATHSIZE+7];
    strncpy(acl,src,MAXPATHSIZE);
    strncat(acl,".access",7);
    // get stat info for the <src>.access file
    // yes I know there's a race condition between lstat and open,
    // but fstat can't determine if the file was a symbolic link
    struct stat aclstat;
    if (lstat(acl,&aclstat) == -1) {
        perror("lstat(acl)");
        exit(1);
    }
    if (!S_ISREG(aclstat.st_mode)) {
        fprintf(stderr,"Access file '%s' is not a regular file\n",acl);
        exit(1);
    }
    if ((aclstat.st_mode & S_IRWXO) != 0) {
        fprintf(stderr,"Access file '%s' has rwx permissions for other users\n",acl);
        exit(1);
    }
    nap* aclnap = newNAP(); // use this for testing access
    if (validateACL(acl,aclnap) == -1) {
        fprintf(stderr,"ACL validation failed\n");
        exit(1);
    }
    // check if the real user listed in the acl
    struct passwd* realpw = getpwuid(uid);
    nap* cursor = aclnap;
    while (cursor != NULL) {
        if (strncmp(realpw->pw_name,cursor->name,MAXNAMESIZE) == 0) {
            if ((cursor->access == 'r') || (cursor->access == 'b')) {
                // user is authorized
                break;
            }
            cursor = NULL;
            break;
        }
        cursor = cursor->next;
    }
    if (cursor == NULL) {
        fprintf(stderr,"User is not authorized to read source file '%s'\n",src);
        exit(1);
    }
    freeNAP(aclnap);
    // get stat info for the src file
    struct stat srcstat;
    if (lstat(src,&srcstat) == -1) {
        perror("lstat(src)");
        exit(1);
    }
    if (!S_ISREG(srcstat.st_mode)) {
        fprintf(stderr,"src file '%s' is not a regular file\n",src);
        exit(1);
    }
    if ((srcstat.st_mode & S_IRWXO) != 0) {
        fprintf(stderr,"src file '%s' has rwx permissions for other users\n",src);
        exit(1);
    }
    // we can open src at this point... finally
    int srcfd = open(src,O_RDONLY);
    if (srcfd == -1) {
        perror("open(srcfd)");
        exit(1);
    }
    if (seteuid(uid) == -1) {// open dest as the real user
        perror("seteuid(uid)");
        exit(1);
    }
    // this open will fail if dest already exists
    // we aren't gonna truncate (O_TRUNC) until we
    // have confirmation from the user that it's ok
    int destfd = open(dest,O_CREAT|O_EXCL|O_WRONLY,0400);
    if ((destfd == -1) && (errno == EEXIST)) {
        // file already existed, so set euid back temporarily
        if (seteuid(euid) == -1) {
            perror("seteuid(euid)");
            exit(1);
        }
        printf("Destination file '%s' already exists\n",dest);
        printf("Type 'yes' and enter if you want to overwrite the file: ");
        int rsize = 5; // enough room for "yes\n\0"
        char response[rsize];
        if (fgets(response,rsize,stdin) == NULL) {
            fprintf(stderr,"Error reading user input\n");
            exit(1);
        }
        if (strncmp(response,"yes",rsize-2) != 0) {
            printf("Exiting without copying\n");
            exit(0);
        }
        // overwrite is granted, try open again on existing file
        if (seteuid(uid) == -1) {// open dest as the real user
            perror("seteuid(uid)");
            exit(1);
        }
        destfd = open(dest,O_TRUNC|O_WRONLY,0400);
        if (destfd == -1) {
            perror("open(dest)");
            exit(1);
        }
    }
    else if ((destfd == -1) && (errno != EEXIST)) {
        perror("open(dest)");
        exit(1);
    }
    if (seteuid(euid) == -1) {// set euid back
        perror("seteuid(euid)");
        exit(1);
    }
    // both src and dest are open, time to copy
    ssize_t readBytes, writeBytes;
    char buffer[BUFSIZE];
    while ((readBytes = read(srcfd,&buffer,BUFSIZE)) > 0) {
        writeBytes = write(destfd,&buffer,readBytes);
        if (writeBytes != readBytes) {
            perror("write()");
            exit(1);
        }
    }
    if (readBytes == -1) {
        perror("read()");
        exit(1);
    }
    // copying done
    close(srcfd);
    close(destfd);
    printf("File '%s' successfully copied to '%s'\n",src,dest);
    exit(0);
}