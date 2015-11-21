#ifndef __SchemaSet
#define __SchemaSet

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
    ~SchemaSet();
    int setSchemasDir(std::string dirname);
    static xmlDocPtr fetchXmlDocPtr(std::string filename);
    xmlDocPtr setSchemaDocFile(std::string filename);
    void printSchemaFilenames();
    void printSchemaDoc(std::string filename);
    xmlNodeSetPtr doXpathQuery(const std::string &schemakey, const std::string &query);
    static void printNodeSet(xmlNodeSetPtr nodeset);
    std::vector<std::string> getPrimaryKey(const std::string &objPath);
    std::vector<std::string> getForeignKey(std::string objPath);

private:
    std::map<std::string, xmlDocPtr> _schemas;
    std::string _schemasdir;
    const std::vector<std::string> listSchemasDir();
    int parseSchemas(const std::vector<std::string>); 
};

#endif
