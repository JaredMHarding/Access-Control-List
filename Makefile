CFLAGS=-c -std=gnu99 -Wall -pedantic

decode: decode.o
	gcc -g decode.o -o decode

decode.o: decode.c
	gcc $(CFLAGS) decode.c

clean:
	rm -f decode *.exe *.o