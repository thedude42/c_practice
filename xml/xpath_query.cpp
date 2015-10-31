#include <iostream>
#include <cstdlib>
#include <string>
#include <assert.h>
#include <GetOpt.h>
#include <vector>
#include <dirent.h>
#include <cctype>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

using namespace std;

const string SCHEMA_BASE = "schema/johnny/";

void print_xpath_nodes(xmlNodeSetPtr, FILE*); 
xmlDocPtr getXmlDoc(char* directory); 
xmlDocPtr getXmlDocFromFile(string filename); 
xmlXPathObjectPtr doXpathQuery(xmlDocPtr doc, const xmlChar* xpath);
void get_primary_key(const char* classpath);
string* usage();
vector<string> getSchemas(string path);

int
main(int argc, char** argv) {
    if((argc < 2)) {
        cerr << *usage() << endl;
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
        cerr << usage() << endl;
        return -1;
    }
    queryStr = argv[index];  
    /* Init libxml */     
    LIBXML_TEST_VERSION

    xmlDocPtr doc = getXmlDoc(directory);
    if (!doc) {
        cerr << "main: problem getting xml from " << argv[1] << endl;
        return(-1);
    }
    
    if (!primaryKey) {
        xmlXPathObjectPtr query_nodes = doXpathQuery(doc, BAD_CAST queryStr);
        
        if (!query_nodes) {
            cerr << "xpath query :" << argv[1] <<  " on document " <<  argv[2] << " returned nothing\n" << endl;
            return(1);
        }
        print_xpath_nodes(query_nodes->nodesetval, stdout);
    }
    else {
        get_primary_key(queryStr);
    }
    return 0;
}

string*
usage() {
    string* str = new string("\
usage:\n\
    xpath_query [-p] query string\n\
    -p: return primary key nodes associated with query string\n");
    return str;
}

xmlDocPtr
getXmlDoc(char* directory) {
    vector<string> files;
    vector<xmlDocPtr> schemaDocs;
    xmlDocPtr next;
    if (directory) {
        files = getSchemas(string(directory));
    }
    else {
        files = getSchemas(SCHEMA_BASE);
    }
    for(vector<string>::iterator it = files.begin(); it != files.end(); ++it) {
        if (it->find(".xml") != string::npos)
            if ((next = getXmlDocFromFile(SCHEMA_BASE+*it)))
                schemaDocs.push_back(next); 
    }
    cout << "schemaDocs has " << schemaDocs.size() << " docs." << endl;
    return getXmlDocFromFile(SCHEMA_BASE+string("ltm.xml"));
}

vector<string> getSchemas(string path) {
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
get_primary_key(const char* classpath) {
    
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
doXpathQuery(xmlDocPtr doc, const xmlChar* xpath) {

    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj;

    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
        cerr << "Error: unable to create new XPath context" << endl;
        return NULL;
    }

    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(xpath, xpathCtx);
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
