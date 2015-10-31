CXX=clang++ 
CPPFLAGS=-c -stdlib=libc++ -Wall 
LDFLAGS=
LDLIBS=-lxml2

I_XML2=-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/libxml2

all: xpath_query

debug: CPPFLAGS += -g
debug: xpath_query

xpath_query: xpath_query.o
	$(CXX) $(LDFLAGS) $(LDLIBS) -o xpath_query xpath_query.o

xpath_query.o: xpath_query.cpp
	$(CXX) $(CPPFLAGS) $(I_XML2)  xpath_query.cpp

clean:
	rm *o xpath_query
