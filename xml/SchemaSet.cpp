#include "SchemaSet.hh"
#include <iostream>

using namespace std;

SchemaSet::SchemaSet() :  _schemasdir("") {
    xmlInitParser();
}

SchemaSet::SchemaSet(string dirname) {
    xmlInitParser();
    if(!setSchemasDir(dirname))
        cout << "No schemas parsed" << endl;
}

SchemaSet::~SchemaSet() {
    for (map<string, xmlDocPtr>::iterator it = _schemas.begin(); it == _schemas.end(); it++) 
        xmlFreeDoc(it->second);
    xmlCleanupParser();
}

int 
SchemaSet::setSchemasDir(string dir) {
    if (!(dir.at(dir.size() - 1) == '/'))
        dir.push_back('/');
    _schemasdir = dir;
    return parseSchemas(listSchemasDir());
}

const vector<string>
SchemaSet::listSchemasDir() {
    DIR*    dir;
    dirent* pdir;
    vector<string> files;
    dir = opendir(_schemasdir.c_str()); //using private
    if (!dir) {
        cerr << "unable to open directory \"" << _schemasdir << "\"" << endl; //private
        exit(-1);
    }
    while ((pdir = readdir(dir))) {
        files.push_back(pdir->d_name);
    }
    closedir(dir);
    return files;
}

int 
SchemaSet::parseSchemas(const vector<string> filelist) {
    xmlDocPtr next;
    size_t offset = 0;
    string key;
    for (vector<string>::const_iterator it = filelist.begin(); it != filelist.end(); ++it) {
        if ((offset = it->find(".xml")) != string::npos) {
            if ((next = fetchXmlDocPtr(_schemasdir+*it))) {
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

xmlXPathObjectPtr
SchemaSet::doXpathQuery(string schema, string query) {
    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj;
    const xmlChar* xpathQuery = reinterpret_cast<const xmlChar*>(query.c_str());
    xpathCtx = xmlXPathNewContext(_schemas[schema]);
    if(!xpathCtx)
        throw "unable to create new XPath context";
    xpathObj = xmlXPathEvalExpression(xpathQuery, xpathCtx);
    if (!xpathObj) {
        xmlXPathFreeObject(xpathObj);
        throw "problem evaluating xpath against " + schema;
    }
    xmlXPathFreeContext(xpathCtx);
    return xpathObj;
}

vector<string>
SchemaSet::getPrimaryKey(string objpath) {
    string queryStr;
    int split = objpath.find("/");
    string key = objpath.substr(0, split);
    cout << "using key \"" << key << "\" for query" << endl;
    string classId = objpath.substr(split+1, objpath.size() - split);
    xmlDocPtr doc = _schemas[key];
    xmlXPathObjectPtr queryResults;
    xmlNodeSetPtr resultSet;
    if (!doc) 
        throw "bad objpath: no matching schema";
    vector<string> retval;
    string URL(reinterpret_cast<const char*>(doc->URL));
    cout << "selected doc: doc->URL=" << URL <<  endl;
    queryStr = "/configurationModule/class[attribute::id = '" + \
            classId + \
            "']/treeIndex[attribute::primaryKey='true']/*/@id";
    cout << "doing query " << queryStr << " against schema keyed on " << key << " for class id=\"" << classId << "\"" << endl;
    queryResults = doXpathQuery(key, queryStr);
    resultSet = queryResults->nodesetval;
    if (!queryResults)
        throw "xpathQuery failed in getPrimaryKey";
    for (int i = 0; i < queryResults->nodesetval->nodeNr; ++i) {
        retval.push_back(string(reinterpret_cast<char *>(resultSet->nodeTab[i]->children->content)));
    }
    xmlXPathFreeObject(queryResults);
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
