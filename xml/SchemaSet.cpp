#include "SchemaSet.hh"
#include <iostream>

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
    directory_iterator dir;
    vector<path> files;
    int numdocs = 0;
    // single file case
    if (is_regular_file(filePath)) {
        xmldoc = fetchXmlDocPtr(filePath.string());
        if (xmldoc) {
            _schemas[filePath.string()] = xmldoc;
            addModule(filePath);
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
            if (is_regular_file(*it)) {
                xmldoc = fetchXmlDocPtr(it->string());
                if (xmldoc) {
                    _schemas[it->string()] = xmldoc;
                    addModule(*it);
                    numdocs++;
                }
            }
            //skip anyhting that isn't a regular file
        }
    }
    return numdocs;
}

void
SchemaSet::addModule(const path &filepath) {
    string basefile = filepath.filename().string();
    int endidx = basefile.rfind(".");
    int basesz = basefile.size();
    if (endidx == string::npos) {
        _modules[basefile] = basefile;
        cout << "DON'T EXPECT THIS" << endl;
    }
    else {
        _modules[basefile.substr(0,basesz-(basesz-endidx))] = filepath.string();
    }
}

string
SchemaSet::getSchemaFromModule(const string modulename) {
    return _modules[modulename];
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
SchemaSet::doXpathQuery(const string &schema, const string &query) {
    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj;
    const xmlChar* xpathQuery = reinterpret_cast<const xmlChar*>(query.c_str());
    xpathCtx = xmlXPathNewContext(_schemas[schema]);
    if(!xpathCtx)
        throw runtime_error("unable to create new XPath context");
    cout << "performing xpath query: " << query << endl;
    xpathObj = xmlXPathEvalExpression(xpathQuery, xpathCtx);
    if (!xpathObj) {
        free(xpathObj);
        throw runtime_error("problem evaluating xpath against " + schema);
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
    cout << "using key \"" << key << "\" for query" << endl;
    string classId = classpath.substr(split+1, classpath.size() - split);
    xmlDocPtr doc = _schemas[getSchemaFromModule(key)];
    xmlNodeSetPtr queryResults;
    if (!doc) 
        throw runtime_error("bad classpath: no matching schema");
    vector<string> retval;
    string URL(reinterpret_cast<const char*>(doc->URL));
    cout << "selected doc: doc->URL=" << URL <<  endl;
    queryStr = "/configurationModule/class[attribute::id = '" + \
            classId + \
            "']/treeIndex[attribute::primaryKey='true']/*/@id";
    cout << "doing query " << queryStr << " against schema keyed on " << key << " for class id=\"" << classId << "\"" << endl;
    queryResults = doXpathQuery(_modules[key], queryStr);
    if (!queryResults)
        throw runtime_error("xpathQuery failed in getPrimaryKey");
    for(int i = 0; i < queryResults->nodeNr; ++i)
        retval.push_back(reinterpret_cast<char *>(queryResults->nodeTab[i]->children->content));
    return retval;
}

void
SchemaSet::printNodeSet(xmlNodeSetPtr nodeset) {
    xmlNodePtr cur;
    int size, i;
    
    size = (nodeset) ? nodeset->nodeNr : 0;
    
    cout << "Result (" << size << " nodeset):" << endl;
    for(i = 0; i < size; ++i) {
        xmlNsPtr ns;
        switch(nodeset->nodeTab[i]->type) {
            case XML_NAMESPACE_DECL :
                ns = (xmlNsPtr)nodeset->nodeTab[i];
                cur = (xmlNodePtr)ns->next;
                if(cur->ns) { 
                    cout << "= namespace \""<< ns->prefix << "\"=\"" << ns->href  << "\" for node " << \
                    cur->ns->href <<":"<<  cur->name << endl;
                }
                else {
                    cout << "= namespace \"" << ns->prefix << "\"=\"" << ns->href << "\" for node " << \
                    cur->name <<  endl; 
                }
                break;
            case XML_ELEMENT_NODE :
                cur = nodeset->nodeTab[i];        
                if (cur->ns) { 
                    cout << "= element node \"" << cur->ns->href << ":" << cur->name << "\"" << endl; 
                }
                else {
                    cout << "= element node \"" << cur->name << "\"" << endl; 
                }
                break;
            case(XML_TEXT_NODE): 
                cur = nodeset->nodeTab[i];        
                if (cur->ns) { 
                    cout << "= text node \"" << cur->ns->href << ":" << cur->content << "\"" << endl; 
                }
                else {
                    cout << "= text node \"" << cur->content << "\"" << endl; 
                }
                break;
            case(XML_ATTRIBUTE_NODE):
                cur = nodeset->nodeTab[i];        
                if (cur->ns) { 
                    cout << "= attr node \"" << cur->ns->href << ":" << cur->content << "\"" << endl; 
                }
                else {
                    cout << "= attr node " << cur->name << " => \"" << cur->children->content << "\"," << endl; 
                }
                break;
            default: 
                cur = nodeset->nodeTab[i];    
                cout << "= node \"" << cur->name << "\": type " << cur->type << endl;
        }
    }
}

void
SchemaSet::printSchemaFilenames() {
    for (map<string, xmlDocPtr>::iterator it=_schemas.begin(); it!=_schemas.end(); ++it)
        cout << it->first << endl;
}
