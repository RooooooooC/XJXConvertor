## 简介

XJXConvertor是XML,JSON和struct之间相互转换的C++转化器

## 依赖:

- [rapidjson](http://rapidjson.org/zh-cn/):RapidJSON 是一个 C++ 的 JSON 解析器及生成器。它的灵感来自 RapidXml
- [rapidxml](http://rapidxml.sourceforge.net/):RapidXml尝试创建尽可能最快的XML解析器，同时保留可用性，可移植性和合理的W3C兼容性。 它是用现代C ++编写的原位解析器，解析速度接近对相同数据执行的strlen函数的解析速度。
- [xml2json](https://github.com/Cheedoong/xml2json):xml2json是第一个精心编写的C ++库，可将XML文档转换为JSON格式
- [x2struct](https://github.com/xyz347/x2struct):用于在C++结构体和json/xml/json/libconfig之间互相转换



### API

- string xml2json(const char *xml_str) : XML格式字符串转化JSON格式字符串
- string xml2json_file(const char *xml_file) : XML文件转化JSON格式字符串
- string json2xml(char *json_str) : JSON格式字符串转化XML格式字符串
- string json2xml_file(char *json_file) : JSON文件转化XML格式字符串
                                
                                               
### 缺陷

rapidxml 和 rapidjson 采用C++异常方式来输出解析XML或JSON错误, XJXConvertor暂不支持C++异常处理, 因此使用之前保证XML和JSON格式正确
                                          

