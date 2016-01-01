#ifndef __SchemaSet
#define __SchemaSet

#include <string>
#include <vector>
#include <map>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <boost/filesystem.hpp>

/* Proposed C interface examples which we need to support:
 *
 * schema_type("/Pool/monitor_rule") : returns Enum, ForeignKey, ValueType
 * get_enum_type("/Profile/type") : returns (for example, profile_type_t)
 * get_enum_values(profile_type_t) : returns table e.g. profile_tcp = 0, profile_udp = 1, profile_http = 2, ...etc
 * get_foreign_key_elements("/Pool/monitor_rule") : returns a list of paths e.g. /MonitorRule/name, /MonitorRule/type, /MonitorRule/pool_name..etc
 * get_primary_key("/Pool") : returns a list of paths e.g. /Pool/name
 * get_reference_paths("/Pool") : returns a list of paths e.g. /Pool/monitor_rule, /Pool/profile
 */

// design goal: provide standard interface like <modiule name>/<class>/<attr>
// for client queries, don't require user to do full schema xpath queries
class SchemaSet {

public:
    SchemaSet();
    SchemaSet(const std::string path);
    ~SchemaSet();
    int addSchemaFile(const std::string filename);
    void printSchemaFilenames() const;
    void printSchemaDoc(const std::string filename) const;//not implemented
    std::vector<std::string> querySchemaModule(std::string modulename, const std::string &querystr) const;
    std::vector<std::string> getPrimaryKey(const std::string &objpath) const;
    std::string getType(const std::string &modulepath) const;
    std::vector<std::string> getForeignKey(const std::string &objpath) const;//not implemented

private:
    std::map<std::string, xmlDocPtr> _schemas; // Document root node attribute "id" => xml document
    std::string getModuleNameFromXmlDoc(const xmlDocPtr doc);
    std::vector<std::string> getXpathQueryResults(xmlDocPtr doc, const std::string &query) const;
    xmlXPathObjectPtr doXpathQuery(xmlDocPtr doc, const std::string &query) const;
    xmlDocPtr fetchXmlDocPtr(const std::string xmldocfilename);
    std::vector<std::string> splitModuleObjectPath(const std::string &modulepath) const;
    std::vector<std::string> parseNodeSet(xmlNodeSetPtr nodeset) const;
    bool addXmlDoc(const boost::filesystem::path &pathname);
};

#endif
