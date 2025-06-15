#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

void fun()
{

    json js;
    js["msgid"] = 1;
    string s = js.dump();
    cout << s << endl;
}

int main()
{
    fun();
    return 0;
}