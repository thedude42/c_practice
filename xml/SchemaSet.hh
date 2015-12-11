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
    std::vector<std::string> getForeignKey(const std::string &objpath);//not implemented

private:
    // change this to be module => xmldoc, based on the result of xpath /configurationModule/@id
    std::map<std::string, xmlDocPtr> _schemas; // file path => xml doc
    std::string getModuleNameFromXmlDoc(xmlDocPtr doc);
    xmlNodeSetPtr doXpathQuery(xmlDocPtr doc, const std::string &query);
    xmlDocPtr fetchXmlDocPtr(const std::string xmldocfilename); // probably not what consumer actually wants/needs
    std::vector<std::string> parseNodeSet(xmlNodeSetPtr nodeset);
    bool addXmlDoc(const boost::filesystem::path &pathname);
};

#endif
