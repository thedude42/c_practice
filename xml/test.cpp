#include "SchemaSet.hh"
#include <iostream>
#include <string>

using namespace std;

const string SCHEMADIR = "schema/johnny";

int main() {
    SchemaSet foo(SCHEMADIR);
    cout << "  [OK] constructed" << endl; 
    cout << "[*] schema names parsed from " << SCHEMADIR << endl;
    foo.printSchemaFilenames();
    cout << "  [OK]" << endl;
   cout << "[*] performing xpath query: \"/configurationModule/class[attribute::id = 'pool']/treeIndex[attribute::primaryKey='true']/*/@id\"" << endl;
    xmlNodeSetPtr result = foo.doXpathQuery("ltm", "/configurationModule/class[attribute::id = 'pool']/treeIndex[attribute::primaryKey='true']/*/@id")->nodesetval;
    string s("name");
    if (!s.compare(reinterpret_cast<char*>(result->nodeTab[0]->children->content))) { 
        foo.printNodeSet(result);
        cout << "  [OK]" << endl;
    }
    else {
        cout << "[XX FAIL XX] : did not get expected result from xpath query" << endl;
    }
    vector<string> pk = foo.getPrimaryKey("ltm/pool");
    if (pk[0] == "name" && pk.size() == 1)
        cout << "  [OK] primary key query for ltm/pool passed" << endl;
    else
        cout << "[XX FAIL XX] : did not get expected return value for primary key of ltm/pool" << endl;
    return 0;
}

