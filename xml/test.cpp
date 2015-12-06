#include "SchemaSet.hh"
#include <iostream>
#include <string>
//#include <boost/test/auto_unit_test.hpp>

using namespace std;

const string SCHEMADIR = "schema/johnny";

bool testXpathQuery(SchemaSet &schemas) {
   cout << "[*] performing xpath query: \"/configurationModule/class[attribute::id = 'pool']/treeIndex[attribute::primaryKey='true']/*/@id\"" << endl;
    xmlNodeSetPtr result = schemas.doXpathQuery("schema/johnny/ltm.xml", "/configurationModule/class[attribute::id = 'pool']/treeIndex[attribute::primaryKey='true']/*/@id");
    string s("name");
    if (!s.compare(reinterpret_cast<char*>(result->nodeTab[0]->children->content))) { 
        schemas.printNodeSet(result);
        cout << "  [OK]" << endl;
        return true;
    }
    else {
        cout << "[XX FAIL XX] : did not get expected result from xpath query" << endl;
        return false;
    }
}

bool testPrimaryKey(SchemaSet &schemas) {
    vector<string> pk = schemas.getPrimaryKey("ltm/pool");
    if (pk[0] == "name" && pk.size() == 1) {
        cout << "  [OK] primary key query for ltm/pool passed" << endl;
        return true;
    }
    else {
        cout << "[XX FAIL XX] : did not get expected return value for primary key of ltm/pool" << endl;
        return false;
    }

}

int main() {
    SchemaSet foo(SCHEMADIR);
    cout << "  [OK] constructed" << endl; 
    cout << "[*] schema names parsed from " << SCHEMADIR << endl;
    foo.printSchemaFilenames();
    cout << "  [OK]" << endl;
    if (!testXpathQuery(foo))
        exit(1);
    if (!testPrimaryKey(foo))
        exit(1);
    return 0;
}

