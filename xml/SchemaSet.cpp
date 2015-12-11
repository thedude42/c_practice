#include "SchemaSet.hh"
#include <iostream>

#include <boost/algorithm/string.hpp>

/*
 * Design refactor:
 * - Offload file/directory validation with boost
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

// think about foreach
SchemaSet::~SchemaSet() {
    for (map<string, xmlDocPtr>::iterator it = _schemas.begin(); it == _schemas.end(); it++) 
        xmlFreeDoc(it->second);
    xmlCleanupParser();
}

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
        for(vector<path>::const_iterator it = files.begin(); 
                                            it != files.end(); ++it) {
            if (is_regular_file(*it)) 
                if (addXmlDoc(*it))
                    numdocs++;
            //skip anyhting that isn't a regular file
        }
    }
    return numdocs;
}

string
SchemaSet::getModuleNameFromXmlDoc(xmlDocPtr doc) {
    xmlNodeSetPtr queryResult = doXpathQuery(doc, "/configurationModule/@id");
    xmlNodePtr moduleName = queryResult->nodeTab[0];
    string retval = reinterpret_cast<char *>(moduleName->children->content);
    return retval;
}

bool
SchemaSet::addXmlDoc(const path &filepath) {
    xmlDocPtr xmldoc = fetchXmlDocPtr(filepath.string());
    string moduleName;
    if (xmldoc) {
        moduleName = getModuleNameFromXmlDoc(xmldoc);
        _schemas[moduleName] = xmldoc;
        return true;
    }
    return false;
}

xmlDocPtr
SchemaSet::fetchXmlDocPtr(const string xmldoc) {
    xmlDocPtr doc = xmlParseFile(xmldoc.c_str());
    if (doc == NULL) {
        xmlFreeDoc(doc);
        // we don't throw so callers don't have to catch
        return(NULL);
    }
    return doc;
}

vector<string>
SchemaSet::querySchemaModule(string modulename, const std::string &querystr) {
    boost::to_upper(modulename);
    xmlDocPtr doc = _schemas[modulename];
    xmlNodeSetPtr queryResults = doXpathQuery(doc, querystr);
    return parseNodeSet(queryResults);
}

 /* SchemaSet::doXpathQuery
  * @ string &schema : valid key in _schemas representing a file system path
  *                    to the schema source file
  * @ string &query : an xpath query string to perform against @schame
  *
  * Accessor method SchemaSet::addModule can be used to convert module
  * name, e.g. "ltm" or "asm", in to the schema file
  *
  * This function leaks one xpathObj per call, so it should probably be a private
  * method, with a public method that handles the cleanup and returns a vector<string>
  */
xmlNodeSetPtr
SchemaSet::doXpathQuery(xmlDocPtr doc, const string &query) {
    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj;
    const xmlChar* xpathQuery = reinterpret_cast<const xmlChar*>(query.c_str());
    xpathCtx = xmlXPathNewContext(doc);
    if(!xpathCtx)
        throw runtime_error("unable to create new XPath context");
    xpathObj = xmlXPathEvalExpression(xpathQuery, xpathCtx);
    if (!xpathObj) {
        free(xpathObj);
        return NULL;
    }
    xmlXPathFreeContext(xpathCtx);
    return xpathObj->nodesetval;
}

/* vector<string>
 * SchemaSet::getPrimaryKey
 * @classpath string of form <module name>/<class id>
 *
 * returns a vecotr<string> with each class attribute that comproses
 * the classpath's "primary key"
 *
 * Need to refactor with private helper method to build query
 */
vector<string>
SchemaSet::getPrimaryKey(const string &classpath) {
    string queryStr;
    int split = classpath.find("/");
    string key = classpath.substr(0, split);
    string classId = classpath.substr(split+1, classpath.size() - split);
    queryStr = "/configurationModule/class[attribute::id = '" + \
            classId + \
            "']/treeIndex[attribute::primaryKey='true']/*/@id";
    return querySchemaModule(key, queryStr);
}

vector<string>
SchemaSet::parseNodeSet(xmlNodeSetPtr nodeset) {
    xmlNodePtr cur;
    xmlNsPtr ns;
    int size, i;
    size = (nodeset) ? nodeset->nodeNr : 0;
    vector<string> retval;
    string node;
    for(i = 0; i < size; ++i) {
        switch(nodeset->nodeTab[i]->type) {
            case XML_NAMESPACE_DECL :
                ns = (xmlNsPtr)nodeset->nodeTab[i];
                cur = (xmlNodePtr)ns->next;
                // convention: xmlns:<namespace>::<result node>
                if(cur->ns) {
                    node += "xmlns:";
                    node += reinterpret_cast<const char *>(ns->prefix);
                    node += "=";
                    node += reinterpret_cast<const char *>(ns->href);
                    node += "::";
                    node += reinterpret_cast<const char *>(cur->ns->href);
                    node += ":";
                    node += reinterpret_cast<const char *>(cur->name);
                }
                else {
                    node = "xmlns:";
                    node += reinterpret_cast<const char *>(ns->prefix);
                    node += "=";
                    node += reinterpret_cast<const char *>(ns->href);
                    node += "::";
                    node += reinterpret_cast<const char *>(cur->name);
                }
                retval.push_back(node);
                break;
            case XML_ELEMENT_NODE :
                cur = nodeset->nodeTab[i];        
                if (cur->ns) { 
                    node = "xmlns:";
                    node += reinterpret_cast<const char *>(cur->ns->href);
                    node += "::";
                    node += reinterpret_cast<const char *>(cur->name);
                }
                else {
                    node = reinterpret_cast<const char *>(cur->name);
                }
                retval.push_back(node);
                break;
            case(XML_TEXT_NODE): 
                cur = nodeset->nodeTab[i];        
                if (cur->ns) { 
                    node = "xmlns:";
                    node += reinterpret_cast<const char *>(cur->ns->href);
                    node += "::";
                    node += reinterpret_cast<const char *>(cur->content); 
                }
                else {
                    node = reinterpret_cast<const char *>(cur->content); 
                }
                retval.push_back(node);
                break;
            case(XML_ATTRIBUTE_NODE):
                cur = nodeset->nodeTab[i];        
                if (cur->ns) { 
                    node = "xmlns:";
                    node += reinterpret_cast<const char *>(cur->ns->href);
                    node += "::@";
                    node += reinterpret_cast<const char *>(cur->name);
                    node += "=";
                    node += reinterpret_cast<const char *>(cur->children->content); 
                }
                else {
                    node = "@";
                    node += reinterpret_cast<const char *>(cur->name);
                    node += "=";
                    node += reinterpret_cast<const char *>(cur->children->content); 
                }
                retval.push_back(node);
                break;
            default: 
                cur = nodeset->nodeTab[i];    
                node = "NodeType:";
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
