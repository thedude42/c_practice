#include "SchemaSet.hh"
#include <iostream>


/*
 * Design refactor:
 * - Offload file/directory validation with boost
 * - Boost "assign" for container insertion/init
 */

using namespace std;

SchemaSet::SchemaSet() :  _schemadirs() {
    xmlInitParser();
}

SchemaSet::SchemaSet(string dirname) {
    xmlInitParser();
    if(!addSchemasDir(dirname))
        cout << "No schemas parsed" << endl;
}

// think about foreach
SchemaSet::~SchemaSet() {
    for (map<string, xmlDocPtr>::iterator it = _schemas.begin(); it == _schemas.end(); it++) 
        xmlFreeDoc(it->second);
}

//TODO: refactor with boost FS stuff
int 
SchemaSet::addSchemasDir(string dir) {
    if (!(dir.at(dir.size() - 1) == '/'))
        dir.push_back('/');
    // we assume listSchemas will bail when this fails
    _schemadirs[dir] = listSchemasDir(dir);
    return parseSchemas(dir);
}

// boost again
const vector<string>
SchemaSet::listSchemasDir(string dirstr) {
    DIR*    dir;
    dirent* pdir;
    vector<string> files;
    dir = opendir(dirstr.c_str()); //using private
    if (!dir) {
        cerr << "unable to open directory \"" << dirstr << "\"" << endl; //private
        exit(-1);
    }
    while ((pdir = readdir(dir))) {
        files.push_back(pdir->d_name);
    }
    closedir(dir);
    return files;
}

int 
SchemaSet::parseSchemas(const string dir) {
    xmlDocPtr next;
    size_t offset = 0;
    string key;
    // foreach refactor
    for(vector<string>::const_iterator it = _schemadirs[dir].begin(); it != _schemadirs[dir].end(); ++it) {
        if ((offset = it->find(".xml")) != string::npos) {
            if ((next = fetchXmlDocPtr(dir+*it))) {
                key = it->substr(0, offset);
                //cout << "adding key: " << key << endl;
                _schemas[key] = next;
            }
            else {
                xmlFreeDoc(next);
            }
        }
    }
    return _schemas.size();
}

xmlDocPtr
SchemaSet::fetchXmlDocPtr(string xmldoc) {
    //xmlInitParser();
    xmlDocPtr doc = xmlParseFile(xmldoc.c_str());
    if (doc == NULL) {
        //cerr << "Error: unable to parse file \"" << xmldoc << "\"" << endl;
        xmlFreeDoc(doc);
        return(NULL);
    }
    return doc;
}

/* Currently we lose track of an xpathObj every time this function is called.
 * Might be better to just return the xpathObj and let the caller deal with it
 */
xmlNodeSetPtr
SchemaSet::doXpathQuery(const string &schema, const string &query) {
    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj;
    const xmlChar* xpathQuery = reinterpret_cast<const xmlChar*>(query.c_str());
    xpathCtx = xmlXPathNewContext(_schemas[schema]);
    if(!xpathCtx)
        throw "unable to create new XPath context";
    xpathObj = xmlXPathEvalExpression(xpathQuery, xpathCtx);
    if (!xpathObj) {
        free(xpathObj);
        throw "problem evaluating xpath against " + schema;
    }
    xmlXPathFreeContext(xpathCtx);
    return xpathObj->nodesetval;
}

vector<string>
SchemaSet::getPrimaryKey(const string &classpath) {
    string queryStr;
    int split = classpath.find("/");
    string key = classpath.substr(0, split);
    cout << "using key \"" << key << "\" for query" << endl;
    string classId = classpath.substr(split+1, classpath.size() - split);
    xmlDocPtr doc = _schemas[key];
    xmlNodeSetPtr queryResults;
    if (!doc) 
        throw "bad classpath: no matching schema";
    vector<string> retval;
    string URL(reinterpret_cast<const char*>(doc->URL));
    cout << "selected doc: doc->URL=" << URL <<  endl;
    queryStr = "/configurationModule/class[attribute::id = '" + \
            classId + \
            "']/treeIndex[attribute::primaryKey='true']/*/@id";
    cout << "doing query " << queryStr << " against schema keyed on " << key << " for class id=\"" << classId << "\"" << endl;
    queryResults = doXpathQuery(key, queryStr);
    if (!queryResults)
        throw "xpathQuery failed in getPrimaryKey";
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