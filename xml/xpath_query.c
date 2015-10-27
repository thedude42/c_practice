#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

int register_namespaces(xmlXPathContextPtr, const xmlChar*); 
void print_xpath_nodes(xmlNodeSetPtr, FILE*); 

xmlDocPtr getXmlDocFromFile(char* filename) {
    xmlInitParser();
    xmlDocPtr doc = xmlParseFile(filename);
    if (doc == NULL) {
        fprintf(stderr, "Error: unable to parse file \"%s\"\n", filename);
        return(NULL);
    }
    return doc;
}

xmlXPathObjectPtr doXpathQuery(xmlDocPtr doc, const xmlChar* xpath, const xmlChar* nsLi) {

    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj;

    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
        fprintf(stderr,"Error: unable to create new XPath context\n");
        return NULL;
    }

    if((nsLi != NULL) && (register_namespaces(xpathCtx, nsLi) < 0)) {
        fprintf(stderr,"Error: failed to register namespaces list \"%s\"\n", nsLi);
        xmlXPathFreeContext(xpathCtx); 
        return(NULL);
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


int main(int argc, char** argv) {
    if((argc < 3) || (argc > 4)) {
        fprintf(stderr, "Error: wrong number of arguments.\n");
        return(-1);
    } 
    
    /* Init libxml */     
    LIBXML_TEST_VERSION

    xmlDocPtr doc = getXmlDocFromFile(argv[1]);
    if (doc == NULL) {
        fprintf(stderr, "main: problem getting xml from %s\n", argv[1]);
        return(-1); 
    }
    
    xmlXPathObjectPtr query_nodes = doXpathQuery(doc, BAD_CAST argv[2], NULL);

    if (query_nodes == NULL) {
        fprintf(stderr, "xpath query %s on document %s returned nothing\n", argv[1], argv[2]);
        return(1);
    }
    print_xpath_nodes(query_nodes->nodesetval, stdout);
    return 0;
}

/**
 * register_namespaces:
 * @xpathCtx:       the pointer to an XPath context.
 * @nsList:     the list of known namespaces in 
 *          "<prefix1>=<href1> <prefix2>=href2> ..." format.
 *
 * Registers namespaces from @nsList in @xpathCtx.
 *
 * Returns 0 on success and a negative value otherwise.
 */
int register_namespaces(xmlXPathContextPtr xpathCtx, const xmlChar* nsList) {
    xmlChar* nsListDup;
    xmlChar* prefix;
    xmlChar* href;
    xmlChar* next;
    
    assert(xpathCtx);
    assert(nsList);

    nsListDup = xmlStrdup(nsList);
    if(nsListDup == NULL) {
        fprintf(stderr, "Error: unable to strdup namespaces list\n");
        return(-1); 
    }
    
    next = nsListDup; 
    while(next != NULL) {
    /* skip spaces */
        while((*next) == ' ') next++;
        if((*next) == '\0') break;

    /* find prefix */
        prefix = next;
        next = (xmlChar*)xmlStrchr(next, '=');
        if(next == NULL) {
            fprintf(stderr,"Error: invalid namespaces list format\n");
            xmlFree(nsListDup);
            return(-1); 
        }
        *(next++) = '\0';   
    
    /* find href */
        href = next;
        next = (xmlChar*)xmlStrchr(next, ' ');
        if(next != NULL) {
            *(next++) = '\0';   
        }   

    /* do register namespace */
        if(xmlXPathRegisterNs(xpathCtx, prefix, href) != 0) {
            fprintf(stderr,"Error: unable to register NS with prefix=\"%s\" and href=\"%s\"\n", prefix, href);
            xmlFree(nsListDup);
            return(-1); 
        }
    }
    
    xmlFree(nsListDup);
    return(0);
}

/**
 * print_xpath_nodes:
 * @nodes:      the nodes set.
 * @output:     the output file handle.
 *
 * Prints the @nodes content to @output.
 */
void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output) {
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
