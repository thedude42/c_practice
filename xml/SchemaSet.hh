#ifndef __SchemaSet
#define __SchemaSet

#include <string>
#include <vector>
#include <map>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <boost/filesystem.hpp>

// design goal: provide standard interface like <modiule name>/<class>/<attr>
// for client queries, don't require user to do full schema xpath queries
class SchemaSet {

public:
    SchemaSet();
    SchemaSet(const std::string path);
    ~SchemaSet();
    int addSchemaFile(const std::string filename);
    static xmlDocPtr fetchXmlDocPtr(const std::string filename); // probably not what consumer actually wants/needs
    void printSchemaFilenames();
    void printSchemaDoc(const std::string filename);//not implemented
    std::string getSchemaFromModule(const std::string modulename);
    xmlNodeSetPtr doXpathQuery(const std::string &schemakey, const std::string &query);
    static void printNodeSet(xmlNodeSetPtr nodeset);
    std::vector<std::string> getPrimaryKey(const std::string &objPath);
    std::vector<std::string> getForeignKey(const std::string &objPath);//not implemented

private:
    // change this to be module => xmldoc, based on the result of xpath /configurationModule/@id
    std::map<std::string, xmlDocPtr> _schemas; // file path => xml doc
    // GET RID OF THIS
    std::map<std::string, std::string> _modules; // module name => file path
    void addModule(const boost::filesystem::path &pathname);
};

#endif
