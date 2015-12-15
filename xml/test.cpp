#include "SchemaSet.hh"
#include <iostream>
#include <string>
//#include <boost/test/auto_unit_test.hpp>

using namespace std;

const string SCHEMADIR = "schema/johnny";

bool
testXpathQuery(SchemaSet &schemas) {
   cout << "[*] performing xpath query: \"/configurationModule/class[attribute::id = 'pool']/treeIndex[attribute::primaryKey='true']/*/@id\"" << endl;
    vector<string> result = schemas.querySchemaModule("LTM", "/configurationModule/class[attribute::id = 'pool']/treeIndex[attribute::primaryKey='true']/*/@id");
    string s("@id=name");
    if (result.size() == 1 && !s.compare(result[0])) { 
        cout << "  [OK] xpath query" << endl;
        return true;
    }
    else {
        cout << "[XX FAIL XX] : did not get expected result from xpath query" << endl;
        cout << "[X RESULT X] : " << result[0] << endl;
        return false;
    }
}

bool
testPrimaryKey(SchemaSet &schemas) {
    vector<string> pk = schemas.getPrimaryKey("ltm/pool");
    if (pk[0] == "@id=name" && pk.size() == 1) {
        cout << "  [OK] primary key query for ltm/pool passed" << endl;
        return true;
    }
    else {
        cout << "[XX FAIL XX] : did not get expected return value for primary key of ltm/pool" << endl;
        cout << "[X RESULT X] : " << pk[0] << endl;
        return false;
    }
}

bool
testGetType(SchemaSet &schemas) {
    string type1 = schemas.getType("ltm/pool/lb_mode");
    string type2 = schemas.getType("ltm/pool/monitor_rule");
    if (type1 == "enum") {
        cout << " [OK] query for type of ltm/pool/lb_mode correctly id'ed as 'enum'" << endl;
        if (type2 == "string") {
            cout << " [OK] query for type of ltm/pool/monitor_rule correctly id'ed as 'string'" << endl;
            return true;
        }
        else {
            cout << "[XX FAIL XX] : did not get correct result for type query ltm/pool/monitor_rule" << endl;
            cout << "[X RESULT X] : " << type2 << endl;
            return false;
        }
    }
    else {
        cout << "[XX FAIL XX] : did not get correct result for type query ltm/pool/lb_mode" << endl;
        cout << "[X RESULT X] : " << type2 << endl;
        return false;
    }
}

int 
main() {
    SchemaSet foo(SCHEMADIR);
    cout << "  [OK] constructed" << endl; 
    cout << "[*] schema names parsed from " << SCHEMADIR << endl;
    foo.printSchemaFilenames();
    cout << "  [OK]" << endl;
    if (!testXpathQuery(foo))
        exit(1);
    if (!testPrimaryKey(foo))
        exit(1);
    if (!testGetType(foo))
        exit(1);
    return 0;
}

