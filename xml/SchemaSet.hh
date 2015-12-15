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
    void printSchemaFilenames();
    void printSchemaDoc(const std::string filename);//not implemented
    std::vector<std::string> querySchemaModule(std::string modulename, const std::string &querystr);
    std::vector<std::string> getPrimaryKey(const std::string &objpath);
    std::string getType(const std::string &modulepath);
    std::vector<std::string> getForeignKey(const std::string &objpath);//not implemented

private:
    std::map<std::string, xmlDocPtr> _schemas; // Document root node attribute "id" => xml document
    std::string getModuleNameFromXmlDoc(xmlDocPtr doc);
    std::vector<std::string> getXpathQueryResults(xmlDocPtr doc, const std::string &query);
    xmlXPathObjectPtr doXpathQuery(xmlDocPtr doc, const std::string &query);
    xmlDocPtr fetchXmlDocPtr(const std::string xmldocfilename);
    std::vector<std::string> parseNodeSet(xmlNodeSetPtr nodeset);
    bool addXmlDoc(const boost::filesystem::path &pathname);
};

#endif
