CFLAGS=-c -std=gnu99 -Wall -pedantic

main: get.o put.o validACL.o
	gcc -g get.o validACL.o -o get
	gcc -g put.o validACL.o -o put

get.o: get.c validACL.h
	gcc $(CFLAGS) get.c

put.o: put.c validACL.h
	gcc $(CFLAGS) put.c

validACL.o: validACL.c validACL.h
	gcc $(CFLAGS) validACL.c

clean:
	rm -f get put *.exe *.o