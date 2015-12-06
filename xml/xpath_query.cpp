#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <assert.h>
#include <GetOpt.h>
#include <cctype>

#include "SchemaSet.hh"

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

using namespace std;

const string SCHEMA_BASE = "schema/johnny/";

void
usage(ostream& out) {
    string str = string("\
usage:\n\
    xpath_query [-p] [-s <schema>] [-d directory] query string \n\
    -p: return primary key nodes associated with query string\n\
    -d: override of default schema directory ("+SCHEMA_BASE+"\n\
    -s: name of schema whose file is in schema directory as <schema>.xml");
     out << str << endl;
}

void
doXpathQuery(SchemaSet &ss, const string &q, const string schema) {
    xmlNodeSetPtr query_nodes = ss.doXpathQuery(schema, q);
    if (!query_nodes) {
        cerr << "xpath query :" << q <<  " on schema " << schema  << " returned nothing" << endl;
    }
    else  {
        SchemaSet::printNodeSet(query_nodes);
    }
}

void
doPrimaryKey(SchemaSet &ss, const string &objpath) {
        cout << "doing search for primary keys" << endl;
        vector<string> pk = ss.getPrimaryKey(objpath);
        cout << "Primary Key Components for object path " << objpath << ":" << endl;
        for_each(pk.begin(), pk.end(), [](string &s){cout << "- " << s << endl;});
}

int
main(int argc, char** argv) {
    if((argc < 2)) {
        usage(cerr);
        return(-1);
    } 
    SchemaSet schemas;
    int primaryKey = 0;
    string dirarg("");
    string queryStr;
    string schema = "ltm";
    int c, index;
    
    opterr = 0;
    while ((c = getopt (argc, argv, "pd:s:")) != -1) {
        switch (c) {
            case 'p':
                primaryKey = 1;
                break;
            case 'd':
                dirarg = string(optarg);
                schemas.addSchemaFile(dirarg);
                break;
            case 's':
                schema = string(optarg);
                break;
            default:;
        }
    }
    //collect trailing non-optional query string arg
    index = optind;
    if (index >= argc) {
        usage(cerr);
        return -1;
    }
    if (dirarg == "")
        schemas.addSchemaFile(SCHEMA_BASE);
    queryStr = string(argv[index]);  
    if (!primaryKey) {
        doXpathQuery(schemas, schema, queryStr);
    }
    else {
        doPrimaryKey(schemas, queryStr);
    }
    return 0;
}


#endif
