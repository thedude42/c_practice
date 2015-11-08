#include <iostream>
#include <cstdlib>
#include <string>
#include <assert.h>
#include <GetOpt.h>
#include <vector>
#include <dirent.h>
#include <cctype>
#include <map>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "SchemaSet.hh"

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

using namespace std;

const string SCHEMA_BASE = "schema/johnny/";

void printXpathNodes(xmlNodeSetPtr, ostream&); 
map<string, xmlDocPtr> loadSchemas(string);
xmlDocPtr getXmlDocFromFile(string filename); 
xmlXPathObjectPtr doXpathQuery(xmlDocPtr, string);
void getPrimaryKey(map<string, xmlDocPtr>, string);
void usage(ostream&);
vector<string> getSchemaFiles(string path);

int
main(int argc, char** argv) {
    if((argc < 2)) {
        usage(cerr);
        return(-1);
    } 
    int primaryKey = 0;
    string queryStr;
    string directory;
    string schema;
    map<string, xmlDocPtr> docs;
    int c, index;
    
    opterr = 0;
    while ((c = getopt (argc, argv, "pd:s:")) != -1) {
        switch (c) {
            case 'p':
                primaryKey = 1;
                break;
            case 'd':
                directory = string(optarg);
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
    queryStr = string(argv[index]);  
    /* Init libxml */     
    LIBXML_TEST_VERSION
    if (directory.size())
        docs = loadSchemas(directory);
    else {
        cout << "WARNING: using default schemas directory because directory was 0 length: " << SCHEMA_BASE << endl;
        docs = loadSchemas(SCHEMA_BASE);
    }
    xmlXPathObjectPtr query_nodes;
    if (!primaryKey) {
        if (schema.size() && docs[schema]) {
            cout << "performing query on schema " << schema << endl;
            query_nodes = doXpathQuery(docs[schema], queryStr);
            if (!query_nodes) {
                cerr << "xpath query :" << queryStr <<  " on document " << docs["ltm"]->URL  << " returned nothing" << endl;
                return(1);
            }
        }
        else if (schema.size() && !docs[schema]) {
            cerr << "schema file " << schema << " not in parsed schema list." << endl;
            exit(-1);
        }
        else {
            for (map<string, xmlDocPtr>::iterator it = docs.begin(); it != docs.end(); ++it) {
                query_nodes = doXpathQuery(docs[schema], queryStr);
                if (query_nodes && query_nodes->nodesetval->nodeNr)
                    break;
            }
        }
        printXpathNodes(query_nodes->nodesetval, cout);
    }
    else {
        cout << "doing search for primary keys" << endl;
        getPrimaryKey(docs, queryStr);
    }
    return 0;
}

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

map<string, xmlDocPtr>
loadSchemas(string directory) {
    vector<string> files;
    map<string, xmlDocPtr> schemaDocs;
    xmlDocPtr next;
    size_t offset = 0;
    string key;
    files = getSchemaFiles(directory);
    for(vector<string>::iterator it = files.begin(); it != files.end(); ++it) {
        if ((offset = it->find(".xml")) != string::npos)
            if ((next = getXmlDocFromFile(SCHEMA_BASE+*it))) {
                key = it->substr(0, offset);
                cout << "adding key: " << key << endl;
                schemaDocs[key] = next;
            }
    }
    cout << "schemaDocs has " << schemaDocs.size() << " docs." << endl;
    return schemaDocs;
}

vector<string>
getSchemaFiles(string path) {
    DIR*    dir;
    dirent* pdir;
    vector<string> files;
    dir = opendir(path.c_str());
    if (!dir) {
        cerr << "unable to open directory \"" << path << "\"" << endl;
        exit(-1);
    }
    while ((pdir = readdir(dir))) {
        files.push_back(pdir->d_name);
    }
    return files;
}

void
getPrimaryKey(map<string, xmlDocPtr> docs, string classpath) {
    string queryStr;
    size_t split = classpath.find("/");
    string key = classpath.substr(0, split);
    cout << "using key \"" << key << "\" for query" << endl;
    string classId = classpath.substr(split+1, classpath.size() - split);
    xmlDocPtr doc;
    xmlXPathObjectPtr queryResults;
    doc = docs[key];

    if (!doc) {
        cerr << key << " is not present in docs<string, xmlDocPtr>, so this is bad" << endl;
        exit(-1);
    }
    string URL(reinterpret_cast<const char*>(doc->URL));
    cout << "selected doc: doc->URL=" << URL <<  endl;
    queryStr = "/configurationModule/class[attribute::id = '" + \
            classId + \
            "']/treeIndex[attribute::primaryKey='true']/*/@id";
    cout << "doing query " << queryStr << " against schema keyed on " << key << " for class id=\"" << classId << "\"" << endl;
    queryResults = doXpathQuery(doc, queryStr);
    printXpathNodes(queryResults->nodesetval, cout);
}

xmlDocPtr
getXmlDocFromFile(string filename) {
    xmlInitParser();
    xmlDocPtr doc = xmlParseFile(filename.c_str());
    if (doc == NULL) {
        cerr << "Error: unable to parse file \"" << filename << "\"" << endl;
        return(NULL);
    }
    return doc;
}

xmlXPathObjectPtr
doXpathQuery(xmlDocPtr doc, string xpath) {

    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj;
    const xmlChar* xpathQuery = reinterpret_cast<const xmlChar*>(xpath.c_str());

    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
        cerr << "Error: unable to create new XPath context" << endl;
        return NULL;
    }

    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(xpathQuery, xpathCtx);
    if(xpathObj == NULL) {
        cerr << "Error: failed evaluating xpath expr => \""  << xpath << "\"" << endl;
        xmlXPathFreeContext(xpathCtx); 
        return(NULL);
    }

    return xpathObj;
}


/**
 * printXpathNodes:
 * @nodes:      the nodes set.
 * @output:     ostream object ref
 *
 * Prints the @nodes content to @output.
 */
void
printXpathNodes(xmlNodeSetPtr nodes, ostream& output) {
    xmlNodePtr cur;
    int size, i;
    
    assert(output);
    size = (nodes) ? nodes->nodeNr : 0;
    
    output << "Result (" << size << " nodes):" << endl;
    for(i = 0; i < size; ++i) {
        assert(nodes->nodeTab[i]);
        xmlNsPtr ns;
        switch(nodes->nodeTab[i]->type) {
            case XML_NAMESPACE_DECL :
                ns = (xmlNsPtr)nodes->nodeTab[i];
                cur = (xmlNodePtr)ns->next;
                if(cur->ns) { 
                    output << "= namespace \""<< ns->prefix << "\"=\"" << ns->href  << "\" for node " << \
                    cur->ns->href <<":"<<  cur->name << endl;
                }
                else {
                    output << "= namespace \"" << ns->prefix << "\"=\"" << ns->href << "\" for node " << \
                    cur->name <<  endl; 
                }
                break;
            case XML_ELEMENT_NODE :
                cur = nodes->nodeTab[i];        
                if (cur->ns) { 
                    output << "= element node \"" << cur->ns->href << ":" << cur->name << "\"" << endl; 
                }
                else {
                    output << "= element node \"" << cur->name << "\"" << endl; 
                }
                break;
            case(XML_TEXT_NODE): 
                cur = nodes->nodeTab[i];        
                if (cur->ns) { 
                    output << "= text node \"" << cur->ns->href << ":" << cur->content << "\"" << endl; 
                }
                else {
                    output << "= text node \"" << cur->content << "\"" << endl; 
                }
                break;
            case(XML_ATTRIBUTE_NODE):
                cur = nodes->nodeTab[i];        
                if (cur->ns) { 
                    output << "= attr node \"" << cur->ns->href << ":" << cur->content << "\"" << endl; 
                }
                else {
                    output << "= attr node " << cur->name << " => \"" << cur->children->content << "\"," << endl; 
                }
                break;
            default: 
                cur = nodes->nodeTab[i];    
                output << "= node \"" << cur->name << "\": type " << cur->type << endl;
        }
    }
}
#endif
