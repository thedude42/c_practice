#include "SchemaSet.hh"
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

/*
 * Design refactor:
 * - Boost "assign" for container insertion/init
 */

using namespace boost::filesystem;
using namespace std;

SchemaSet::SchemaSet() :  _schemas() {
    xmlInitParser();
}

SchemaSet::SchemaSet(const string path) {
    xmlInitParser();
    if(!addSchemaFile(path))
        cout << "No schemas parsed" << endl;
}

SchemaSet::~SchemaSet() {
    for_each(_schemas.begin(), _schemas.end(), [](pair<string, xmlDocPtr> doc) {
        xmlFreeDoc(doc.second);
    });
    xmlCleanupParser();
}

/* SchemaSet::addSchemaFile
 * @filename : string representing file or directory path
 *
 * Mutator method for updating the state of the SchemaSet attribute _schemas.
 * Input is a valid filename or directory, and in the directory case only the containing files
 * are parsed as schema documents and any containing directories are ignored.
 *
 * Returns the number of documents successfully parsed.
 */
int
SchemaSet::addSchemaFile(const string fileName) {
    path filePath(fileName);
    if (!exists(filePath)) {
        return 0;
    }
    xmlDocPtr xmldoc;
    vector<path> files;
    int numdocs = 0;
    // single file case
    if (is_regular_file(filePath)) {
        xmldoc = fetchXmlDocPtr(filePath.string());
        if (xmldoc) {
            _schemas[filePath.string()] = xmldoc;
            if (addXmlDoc(filePath))
                numdocs = 1;
        }
    }
    // directory case
    else if (is_directory(filePath)) {
        copy(directory_iterator(filePath), directory_iterator(), 
             back_inserter(files));
    }
    // guard for directory case
    if (files.size() > 0) {
        //for (vector<path>::const_iterator it = files.begin(); 
        //                                    it != files.end(); ++it) {
        for (auto file : files) {
            if (is_regular_file(file)) 
                if (addXmlDoc(file))
                    numdocs++;
            //skip anyhting that isn't a regular file
        }
    }
    return numdocs;
}

/* SchemaSet::addXmlDoc
 * @filepath : boost::filesystem::path object representing the path to the xml documnt
 *
 * Mutator convenience method to add xml doc to _schemas
 */
bool
SchemaSet::addXmlDoc(const path &filepath) {
    string moduleName;
    xmlDocPtr xmldoc = fetchXmlDocPtr(filepath.string());
    if (xmldoc) {
        moduleName = getModuleNameFromXmlDoc(xmldoc);
        _schemas[moduleName] = xmldoc;
        return true;
    }
    return false;
}

/* SchemaSet::getModuleNameFromXmlDoc
 * @doc : xml document pointer
 *
 * Helper method that gets the schema module name from the xml document parameter
 */
string
SchemaSet::getModuleNameFromXmlDoc(xmlDocPtr doc) {
    vector<string> queryResult = getXpathQueryResults(doc, "/configurationModule/@id");
    string modulename = queryResult[0];
    return modulename.substr(4, modulename.size());
}

/* SchemaSet::fetchXmlDocPtr
 * @xmldocfilename : file name used to create xmlDocPtr
 *
 * Convenience function for calling libxml2 function xmlParseFile and return new xmlDocPtr.
 * Returns NULL on failure of xmlParseFile
 */
xmlDocPtr
SchemaSet::fetchXmlDocPtr(const string xmldocfilename) {
    xmlDocPtr doc = xmlParseFile(xmldocfilename.c_str());
    if (doc == NULL) {
        xmlFreeDoc(doc);
        // we don't throw so callers don't have to catch
        return(NULL);
    }
    return doc;
}

/* SchemaSet::querySchemaModule
 * @modulename : module name string expected to be a key of _schemas
 * @querystr : query to perform against xmldoc associated with @modulename
 */
vector<string>
SchemaSet::querySchemaModule(string modulename, const std::string &querystr) {
    boost::to_upper(modulename);
    xmlDocPtr doc = _schemas[modulename];
    return getXpathQueryResults(doc, querystr);
}

/* SchemaSet::getXpathQueryResults
 * @doc : document to be queried
 * @query : the query
 *
 * This helper method handles the conversion between xmlXPathObjectPtr and vector<string>,
 * performing cleanup of the xmlXPathObjectPtr memory after the query is complete
 */
vector<string>
SchemaSet::getXpathQueryResults(xmlDocPtr doc, const string &query) {
    xmlXPathObjectPtr queryResults = doXpathQuery(doc,query);
    vector<string> parsedResults = parseNodeSet(queryResults->nodesetval);
    xmlXPathFreeObject(queryResults);
    return parsedResults;
}


 /* SchemaSet::doXpathQuery
  * @ doc : document to query 
  * @ string &query : an xpath query string to perform against @doc
  *
  * Implementation method for performing xpath query with libxml2 functions
  *
  * Intended to be used by getXpathQueryResults, which cleans up the xmlXPathObjectPtr memory
  */
xmlXPathObjectPtr
SchemaSet::doXpathQuery(xmlDocPtr doc, const string &query) {
    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj;
    const xmlChar* xpathQuery = reinterpret_cast<const xmlChar*>(query.c_str());
    xpathCtx = xmlXPathNewContext(doc);
    // This condition is really bad and needs investigation
    if(!xpathCtx)
        throw runtime_error("unable to create new XPath context");
    xpathObj = xmlXPathEvalExpression(xpathQuery, xpathCtx);
    if (!xpathObj) {
        free(xpathObj);
        return NULL;
    }
    xmlXPathFreeContext(xpathCtx);
    return xpathObj;
}

std::vector<string>
splitModuleObjectPath(const std::string &modulepath) {
    vector<string> retval;
    boost::split(retval, modulepath, boost::is_any_of("/"));
    return retval;
}

/*  SchemaSet::getPrimaryKey
 * @classpath string of form <module name>/<class id>
 *
 * Builds the proper xpath query to return a class's private key stuff.
 * Returns a vecotr<string> with each class attribute that comprises
 * the classpath's "primary key"
 *
 */
vector<string>
SchemaSet::getPrimaryKey(const string &classpath) {
    vector<string> pathparts = splitModuleObjectPath(classpath);
    string querystr = "/configurationModule/class[attribute::id = '" + \
            pathparts[1] + \
            "']/treeIndex[attribute::primaryKey='true']/*/@id";
    return querySchemaModule(pathparts[0], querystr);
}

string
SchemaSet::getType(const std::string &modulepath) {
    vector<string> pathparts = splitModuleObjectPath(modulepath);
    string querystr = "/configurationModule/class[attribute::id = '" + \
                      pathparts[1] + "']/atom[attribute::id = '" + pathparts[2] + "']/@type";
    vector<string> query_results = querySchemaModule(pathparts[0], querystr);
    if (query_results.size() == 0)
        return "";
    string type = query_results[0].substr(6, query_results[0].size());
    querystr = "/configurationModule/enum[attribute::id= '" + type + "']";
    query_results = querySchemaModule(pathparts[0], querystr);
    if (query_results.size() == 0)
        return type;
    return query_results[0];
}

/* SchemaSet::parseNodeSet
 *
 * @nodeset : pointer to libxml2 structure containing results of xpath query
 *
 * This method enforces our return value convention where nodes are returned like so, witout <> characters:
 * 
 * @@ If there is a namespace involved, prefix the string with:
 *   xmlns:<ns href>::
 * 
 * Type XML_NAMESPACE_DECL (with NO namespace)
 *   xmlns:<ns prefix>=<ns href>::<node name>
 * Type XML_ELEMENT_NODE
 *   <node name>
 * Type XML_TEXT_NODE
 *   <node content>
 * Type XML_ATTRIBUTE_NODE
 *   @<attribute name>=<attribute value>
 * All other types:
 *   NodeType:<type>=<node name>
 */
vector<string>
SchemaSet::parseNodeSet(xmlNodeSetPtr nodeset) {
    xmlNodePtr cur;
    xmlNsPtr ns;
    int size, i;
    size = (nodeset) ? nodeset->nodeNr : 0;
    vector<string> retval;
    string node;
    for(i = 0; i < size; ++i) {
        node = "";
        switch(nodeset->nodeTab[i]->type) {
            // TODO: this is a bit goofy and I'm not sure how best to render the namespace information
            case XML_NAMESPACE_DECL :
                ns = (xmlNsPtr)nodeset->nodeTab[i];
                cur = (xmlNodePtr)ns->next;
                if(cur->ns) {
                    node += "xmlns:";
                    node += reinterpret_cast<const char *>(ns->prefix);
                    node += "=";
                    node += reinterpret_cast<const char *>(ns->href);
                    node += "::";
                }
                node += "xmlns:";
                node += reinterpret_cast<const char *>(cur->name);
                retval.push_back(node);
                break;
            case XML_ELEMENT_NODE :
                cur = nodeset->nodeTab[i];        
                if (cur->ns) { 
                    node += "xmlns:";
                    node += reinterpret_cast<const char *>(cur->ns->prefix);
                    node += "=";
                    node += reinterpret_cast<const char *>(cur->ns->href);
                    node += "::";
                }
                node += reinterpret_cast<const char *>(cur->name);
                retval.push_back(node);
                break;
            case(XML_TEXT_NODE): 
                cur = nodeset->nodeTab[i];        
                if (cur->ns) { 
                    node += "xmlns:";
                    node += reinterpret_cast<const char *>(cur->ns->prefix);
                    node += "=";
                    node += reinterpret_cast<const char *>(cur->ns->href);
                    node += "::";
                }
                node += reinterpret_cast<const char *>(cur->content); 
                retval.push_back(node);
                break;
            case(XML_ATTRIBUTE_NODE):
                cur = nodeset->nodeTab[i];        
                if (cur->ns) { 
                    node += "xmlns:";
                    node += reinterpret_cast<const char *>(cur->ns->prefix);
                    node += "=";
                    node += reinterpret_cast<const char *>(cur->ns->href);
                    node += "::";
                }
                node += "@";
                node += reinterpret_cast<const char *>(cur->name);
                node += "=";
                node += reinterpret_cast<const char *>(cur->children->content); 
                retval.push_back(node);
                break;
            default: 
                cur = nodeset->nodeTab[i];    
                node += "NodeType:";
                node += reinterpret_cast<const char *>(cur->type);
                node += " = ";
                node += reinterpret_cast<const char *>(cur->name);
                retval.push_back(node);
        }
    }
    return retval;
}

void
SchemaSet::printSchemaFilenames() {
    for_each(_schemas.begin(), _schemas.end(), [](pair<string, xmlDocPtr> s){
        cout << s.first << endl;
    });
}
