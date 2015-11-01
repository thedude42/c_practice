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

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

using namespace std;

const string SCHEMA_BASE = "schema/johnny/";

void print_xpath_nodes(xmlNodeSetPtr, FILE*); 
map<string, xmlDocPtr> loadSchemas(char* directory);
xmlDocPtr getXmlDocFromFile(string filename); 
xmlXPathObjectPtr doXpathQuery(xmlDocPtr, string);
void getPrimaryKey(map<string, xmlDocPtr>, string);
void usage(int);
vector<string> getSchemaFiles(string path);

int
main(int argc, char** argv) {
    if((argc < 2)) {
        usage(2);
        return(-1);
    } 
    int primaryKey = 0;
    char *queryStr = NULL;
    char *directory = NULL;
    int c, index;
    
    opterr = 0;
    while ((c = getopt (argc, argv, "pd:")) != -1)
        switch (c) {
            case 'p':
                primaryKey = 1;
                break;
            case 'd':
                directory = optarg;
                break;
            default:;
        }
    
    index = optind;
    if (index > argc) {
        usage(2);
        return -1;
    }
    queryStr = argv[index];  
    /* Init libxml */     
    LIBXML_TEST_VERSION

    map<string, xmlDocPtr> docs = loadSchemas(directory);
    
    if (!primaryKey) {
        cout << "performing query on schema 'ltm'" << endl;
        xmlXPathObjectPtr query_nodes = doXpathQuery(docs["ltm"], string(queryStr));
        
        if (!query_nodes) {
            cerr << "xpath query :" << queryStr <<  " on document " << docs["ltm"]->URL  << " returned nothing\n" << endl;
            return(1);
        }
        print_xpath_nodes(query_nodes->nodesetval, stdout);
    }
    else {
        cout << "doing search for primary keys" << endl;
        getPrimaryKey(docs, string(queryStr));
    }
    return 0;
}

void
usage(int descriptor) {
    string str = string("\
usage:\n\
    xpath_query [-p] query string\n\
    -p: return primary key nodes associated with query string\n");
    if (descriptor == 1)
        cout << str << endl;
    else
        cerr << str << endl;
}

map<string, xmlDocPtr>
loadSchemas(char* directory) {
    vector<string> files;
    map<string, xmlDocPtr> schemaDocs;
    xmlDocPtr next;
    size_t offset = 0;
    string key;
    if (directory) {
        files = getSchemaFiles(string(directory));
    }
    else {
        files = getSchemaFiles(SCHEMA_BASE);
    }
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
    string classId = classpath.substr(split+1, classpath.size() - split);
    xmlDocPtr doc;
    xmlXPathObjectPtr queryResults;
    
    doc = docs[key];
    queryStr = "/configurationModule/class[attribute::id = '" + \
            classId + \
            "']/treeIndex[attribute::primaryKey='true']/*/@id";
    cout << "doing query " << queryStr << " against schema keyed on " << key << " for class id=\"" << classId << "\"" << endl;
    queryResults = doXpathQuery(doc, queryStr);
    print_xpath_nodes(queryResults->nodesetval, stdout);
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
 * print_xpath_nodes:
 * @nodes:      the nodes set.
 * @output:     the output file handle.
 *
 * Prints the @nodes content to @output.
 */
void
print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output) {
    xmlNodePtr cur;
    int size;
    int i;
    
    assert(output);
    size = (nodes) ? nodes->nodeNr : 0;
    
    fprintf(output, "Result (%d nodes):\n", size);
    for(i = 0; i < size; ++i) {
        assert(nodes->nodeTab[i]);
    
        if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
            xmlNsPtr ns;
        
            ns = (xmlNsPtr)nodes->nodeTab[i];
            cur = (xmlNodePtr)ns->next;
            if(cur->ns) { 
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s:%s\n", 
                ns->prefix, ns->href, cur->ns->href, cur->name);
            }
            else {
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s\n", 
                ns->prefix, ns->href, cur->name);
            }
        }
        else if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
            cur = nodes->nodeTab[i];        
            if (cur->ns) { 
                fprintf(output, "= element node \"%s:%s\"\n", 
                    cur->ns->href, cur->name);
            }
            else {
                fprintf(output, "= element node \"%s\"\n", 
                    cur->name);
            }
        }
        else if (nodes->nodeTab[i]->type == XML_TEXT_NODE) {
            cur = nodes->nodeTab[i];        
            if (cur->ns) { 
                fprintf(output, "= element node \"%s:%s\"\n", 
                    cur->ns->href, cur->content);
            }
            else {
                fprintf(output, "= element node \"%s\"\n", 
                    cur->content);
            }
            
        }
        else if (nodes->nodeTab[i]->type == XML_ATTRIBUTE_NODE) {
            cur = nodes->nodeTab[i];        
            if (cur->ns) { 
                fprintf(output, "= element node \"%s:%s\"\n", 
                    cur->ns->href, cur->content);
            }
            else {
                fprintf(output, "= element attr node %s => \"%s\"\n", 
                    cur->name, cur->children->content);
            }
        }
        else {
            cur = nodes->nodeTab[i];    
            fprintf(output, "= node \"%s\": type %d\n", cur->name, cur->type);
        }
    }
}
#endif
