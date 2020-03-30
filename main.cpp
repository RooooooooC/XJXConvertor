#include <iostream>
#include <fstream>

#include "XJXConvertor/xjxconvertor.hpp"

int main(void)
{
    XJXConvertor xjxConvertor;
#if 1
    std::string xml = xjxConvertor.json2xml_file("1212-8ying1.json");
    cout << xml << endl;
#else
    std::string json = xjxConvertor.xml2json_file("1212-8ying.xml");
    cout << json << endl;

    std::ofstream ofs;
    ofs.open("1212-8ying1.json");
    ofs << json.data();
    ofs.close();
#endif
    return 0;
}
