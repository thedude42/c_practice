#include "SchemaSet.hh"
#include <iostream>

using namespace std;

SchemaSet::SchemaSet() {
    map <string, xmlDocPtr> _schemas;
    string _schemadir("");
}

SchemaSet::SchemaSet(string dirname) : _schemadir(dirname) {
    if (!parseSchemas(listSchemaDir())) 
        cout << "No schemas parsed" << endl;
}

vector<string>*
SchemaSet::listSchemaDir() {
    DIR*    dir;
    dirent* pdir;
    vector<string>* files = new vector<string>;
    dir = opendir(_schemadir.c_str()); //using private
    if (!dir) {
        cerr << "unable to open directory \"" << _schemadir << "\"" << endl; //private
        exit(-1);
    }
    while ((pdir = readdir(dir))) {
        files->push_back(pdir->d_name);
    }
    return files;
}

int 
SchemaSet::parseSchemas(vector<string>* filelist) {
    xmlDocPtr next;
    size_t offset = 0;
    string key;
    int retval = 0;
    for(vector<string>::iterator it = filelist->begin(); it != filelist->end(); ++it) {
        if ((offset = it->find(".xml")) != string::npos)
            if ((next = fetchXmlDocPtr(_schemadir+*it))) {
                key = it->substr(0, offset);
                //cout << "adding key: " << key << endl;
                _schemas[key] = next;
                retval++;
            }
    }
    return retval;
}

xmlDocPtr
SchemaSet::fetchXmlDocPtr(string xmldoc) {
    xmlInitParser();
    xmlDocPtr doc = xmlParseFile(xmldoc.c_str());
    if (doc == NULL) {
        cerr << "Error: unable to parse file \"" << xmldoc << "\"" << endl;
        return(NULL);
    }
    return doc;
}
