#ifndef __SchemaSet

#include <string>
#include <vector>
#include <map>
#include <dirent.h>

#include <libxml/tree.h>
#include <libxml/xpath.h>

class SchemaSet {

public:
    SchemaSet();
    SchemaSet(std::string dirname);
    std::vector<xmlDocPtr> setSchemasDir(std::string dirname);
    xmlDocPtr setSchemaDocFile(std::string filename);
    xmlXPathObjectPtr doXpathQuery(std::string query);
    std::vector<std::string> getPrimaryKey(std::string objPath);
    void printSchemaFilenames();
    void printSchemaDoc(std::string filename);
    static xmlDocPtr fetchXmlDocPtr(std::string filename);

private:
    std::vector<std::map<std::string, xmlDocPtr> > _schemas;
};

#endif
