#ifndef __SchemaSet
#define __SchemaSet

#include <string>
#include <vector>
#include <map>
#include <dirent.h>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <boost/filesystem.hpp>

class SchemaSet {

public:
    SchemaSet();
    SchemaSet(std::string dirname);
    ~SchemaSet();
    int addSchemaFile(const std::string filename);
    static xmlDocPtr fetchXmlDocPtr(std::string filename);
    void printSchemaFilenames();
    void printSchemaDoc(std::string filename);
    std::string getSchemaFromModule(const std::string modulename);
    xmlNodeSetPtr doXpathQuery(const std::string &schemakey, const std::string &query);
    static void printNodeSet(xmlNodeSetPtr nodeset);
    std::vector<std::string> getPrimaryKey(const std::string &objPath);
    std::vector<std::string> getForeignKey(std::string objPath);

private:
    std::map<std::string, xmlDocPtr> _schemas;
    std::map<std::string, std::string> _modules;
    void addModule(const boost::filesystem::path &pathname);
    int parseSchemas(const std::string); 
};

#endif
