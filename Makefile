CC=g++
CFLAGS=-g -Wall -Werror -std=c++11
INCLUDE_CURL=-I/usr/include/curl
INCLUDE_LIBXML=-I/usr/include/libxml2
LINK_CURL=-lcurl
LINK_LIBXML=-lxml2

EXECUTABLE=main
OBJECT_FILES=DateTimeStamper.o
DSYM_FOLDER=main.dSYM

all: main

main: DateTimeStamper.o main.cpp
	$(CC) $(CFLAGS) $(INCLUDE_CURL) $(INCLUDE_LIBXML) $(LINK_CURL) $(LINK_LIBXML) $(OBJECT_FILES) main.cpp -o main

DateTimeStamper.o: DateTimeStamper.cpp
	$(CC) $(CFLAGS) -c DateTimeStamper.cpp

clean:
	rm -rfv $(EXECUTABLE) $(OBJECT_FILES) $(DSYM_FOLDER)
