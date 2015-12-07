#ifndef __SchemaSet
#define __SchemaSet

#include <string>
#include <vector>
#include <map>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <boost/filesystem.hpp>

class SchemaSet {

public:
    SchemaSet();
    SchemaSet(const std::string path);
    ~SchemaSet();
    int addSchemaFile(const std::string filename);
    static xmlDocPtr fetchXmlDocPtr(const std::string filename);
    void printSchemaFilenames();
    void printSchemaDoc(const std::string filename);//not implemented
    std::string getSchemaFromModule(const std::string modulename);
    xmlNodeSetPtr doXpathQuery(const std::string &schemakey, const std::string &query);
    static void printNodeSet(xmlNodeSetPtr nodeset);
    std::vector<std::string> getPrimaryKey(const std::string &objPath);
    std::vector<std::string> getForeignKey(const std::string &objPath);//not implemented

private:
    std::map<std::string, xmlDocPtr> _schemas;
    std::map<std::string, std::string> _modules;
    void addModule(const boost::filesystem::path &pathname);
};

#endif
