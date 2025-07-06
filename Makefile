#------------------------------------------------------------------------------
# Makefile for HTTP Server
#
# make                   makes httpserver
# make clean             removes all binaries
#------------------------------------------------------------------------------

CC = clang
CFLAGS = -pthread -g -Wall -Werror -Wextra -pedantic
BASE_SOURCES   = httphelpers.c List.c queue.c
BASE_OBJECTS   = httphelpers.o List.o queue.o
HEADERS        = httphelpers.h List.h queue.h

all: httpserver

httpserver: httpserver.o $(BASE_OBJECTS)
	$(CC) -o httpserver -pthread httpserver.o $(BASE_OBJECTS)

httpserver.o: httpserver.c $(HEADERS)
	$(CC) $(CFLAGS) -c httpserver.c

$(BASE_OBJECTS) : $(BASE_SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $(BASE_SOURCES)

clean:
	rm -f httpserver httpserver.o $(BASE_OBJECTS)
	