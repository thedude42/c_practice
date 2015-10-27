CC=clang 

LDFLAGS+=-lxml2

CFLAGS=-c -Wall

I_XML2=-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/libxml2

all: xpath_query

debug: CFLAGS += -g
debug: xpath_query

xpath_query: xpath_query.o

xpath_query.o: xpath_query.c
	$(CC) $(CFLAGS) $(I_XML2) xpath_query.c

clean:
	rm *o xpath_query
