# Jared Harding | CS 483 | Project 1 #

## Description ##

This program can be used to give users access to files they do not have regular permissions for.

A main user can take a file they want people to have access to, `basename.ext`, create an Access Control List (ACL) file, `basename.ext.access`, with a list of all of the authorized users that are allowed to read from or write to `basename.ext`.

Other users, specified in the ACL, will be able to have access to your file by using the `get` and `put` binaries.

### Get ###

`get` can be used from the command line like this:

`$ ./get source destination`

• source - the file you want to copy from (this has an ACL file)
• destination - the file you want to copy to, this can be a new or already existing file of the user's

### Put ###

`put` can be used from the command line like this:

`$ ./put source destination`

• source - the file you want to copy from, this can be a new or already existing file of the user's
• destination - the file you want to copy to (this has an ACL file)

### How To Format An ACL File ###

Entries in the ACL file each contain two components separated by whitespace (space, tab). The first component, which may be preceded by whitespace, is a single userid (alphanumeric value, e.g. "jharding"). The second is a single character r, w, or b, indicating read, write, or both read and write access, respectively, for the user with the corresponding userid. This second component may be followed by whitespace. Lines beginning with the character '#' are comments. No blank lines are allowed.

## Notes About The Implementation ##

I used `lstat()` for determining whether files were symbolic links, since `fstat()` can not tell whether the opened file descriptor was a symbolic link. This creates a race condition from the time `lstat()` is used to the moment the file is actually opened with `open()`.