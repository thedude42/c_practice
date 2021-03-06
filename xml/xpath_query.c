#include <iostream>
#include <cstdlib>
#include <string>
#include <assert.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

const char SCHEMA_BASE[] = "schema/johnny/";

int register_namespaces(xmlXPathContextPtr, const xmlChar*); 
void print_xpath_nodes(xmlNodeSetPtr, FILE*); 
xmlDocPtr getXmlDocFromFile(char* filename); 
xmlXPathObjectPtr doXpathQuery(xmlDocPtr doc, const xmlChar* xpath);
void get_primary_key(const char* classpath);

int
main(int argc, char** argv) {
    if((argc < 3) || (argc > 4)) {
        cerr << "Error: wrong number of arguments." << endl;
        return(-1);
    } 
    
    /* Init libxml */     
    LIBXML_TEST_VERSION

    xmlDocPtr doc = getXmlDocFromFile(argv[1]);
    if (!doc) {
        cerr <<"main: problem getting xml from " << argv[1] << endl;
        return(-1);
    }
    
    xmlXPathObjectPtr query_nodes = doXpathQuery(doc, BAD_CAST argv[2]);

    if (!query_nodes) {
        cerr << "xpath query :" << argv[1] <<  " on document " <<  argv[2] << " returned nothing\n" << endl;
        return(1);
    }
    print_xpath_nodes(query_nodes->nodesetval, stdout);
    return 0;
}

void
get_primary_key(const char* classpath) {
    
}

xmlXPathObjectPtr
doXpathQuery(xmlDocPtr doc, const xmlChar* xpath) {

    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj;

    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
        fprintf(stderr,"Error: unable to create new XPath context\n");
        return NULL;
    }

    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(xpath, xpathCtx);
    if(xpathObj == NULL) {
        fprintf(stderr,"Error: failed evaluating xpath expr => \"%s\"\n", xpath);
        xmlXPathFreeContext(xpathCtx); 
        return(NULL);
    }

    return xpathObj;
}

xmlDocPtr
getXmlDocFromFile(char* filename) {
    xmlInitParser();
    xmlDocPtr doc = xmlParseFile(filename);
    if (doc == NULL) {
        fprintf(stderr, "Error: unable to parse file \"%s\"\n", filename);
        return(NULL);
    }
    return doc;
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
