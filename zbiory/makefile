CC=gcc
CFLAGS=-Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2 -g
LDFLAGS=-shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

all: libseq.so seq_example 

memory_tests.o: memory_tests.c memory_tests.h
seq.o: seq.c seq.h
seq_example.o: seq_example.c seq.h memory_tests.h

libseq.so: seq.o memory_tests.o
	$(CC) $(LDFLAGS) -o libseq.so seq.o memory_tests.o

seq_example: seq_example.c libseq.so
	$(CC) -o seq_example seq_example.c -L. -lseq

clean:
	rm -f *.o libseq.so seq_example memory_tests