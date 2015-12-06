CXX=clang++ 
CPPFLAGS=-c -std=c++11 -stdlib=libc++ -Wall 
LDLIBS=-lxml2
LDLIBS+= -lboost_filesystem
LDLIBS+= -lboost_system
LDLIBS+= -lboost_unit_test_framework
LDFLAGS= -L/usr/local/opt/libxml2/lib

INCLUDE= -I/usr/local/opt/libxml2/include/libxml2
#I_XML2=-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/libxml2

all: xpath_query

debug: CPPFLAGS += -g
debug: xpath_query test
debug: test

test: SchemaSet.o test.o
	$(CXX) $(LDFLAGS) $(LDLIBS) -o test SchemaSet.o test.o
	./test

test.o: test.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDE)  test.cpp

unit_tests: SchemaSet.o unit_tests.o
	$(CXX) $(LDFLAGS) $(LDLIBS) -o unit_tests SchemaSet.o unit_tests.o

unit_tests.o: unit_tests.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDE)  unit_tests.cpp

xpath_query: xpath_query.o SchemaSet.o
	$(CXX) $(LDFLAGS) $(LDLIBS) -o xpath_query xpath_query.o SchemaSet.o

xpath_query.o: xpath_query.cpp
	$(CXX) $(CPPFLAGS) xpath_query.cpp $(INCLUDE)  

SchemaSet.o: SchemaSet.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDE) SchemaSet.cpp

clean:
	rm *o xpath_query test
