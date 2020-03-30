#ifndef XJXCONVERTOR_HPP
#define XJXCONVERTOR_HPP

#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>

#include "../rapidxml/rapidxml.hpp"
#include "../rapidxml/rapidxml_utils.hpp"
#include "../rapidxml/rapidxml_print.hpp"

#include "../rapidjson/document.h"
#include "../rapidjson/prettywriter.h"
#include "../rapidjson/encodedstream.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/reader.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/filereadstream.h"
#include "../rapidjson/filewritestream.h"
#include "../rapidjson/error/en.h"

using namespace std;
using namespace rapidxml;
using namespace rapidjson;


class XJXConvertor {

private:
    /*
     * json2xml variant
     */
    typedef enum XJX_XMLTYPE {
        XJX_ATTR_XMLTYPE = 0, //@
        XJX_TEXT_XMLTYPE, //#
        XJX_SPECTEXT_XMLTYPE,//#text
        XJX_NORNALTEXT_XMLTYPE
    }XJX_XMLTYPE;

    int rootNodeArrFlags = 0; //根节点的兄弟节点不能存在数组节点,否则xml2json不能等价转换
    const char attrFlags ='@';
    const char textChFlags = '#';
    const char *textStrFlags = "#text";
    xml_document<> j2x_xmlDoc;
    Document j2x_jsonDoc;

    /*
     * xml2json variant
     */
    const char *xml2json_text_additional_name = "#text";
    const char *xml2json_attribute_name_prefix = "@";
    const bool xml2json_numeric_support = true;

    xml_document<> x2j_xmldoc;
    Document x2j_jsonDoc;


    /*
     * json2xml function
     */
    XJX_XMLTYPE parse_xjx_objName(const char *objName)
    {
        int cmp;

        if (objName[0] == attrFlags)
            return XJX_ATTR_XMLTYPE;

        cmp = strncmp(objName, textStrFlags, strlen(textStrFlags));
        if (objName[0] == textChFlags && cmp) {
            return XJX_TEXT_XMLTYPE;
        }

        if (strlen(objName) == strlen(textStrFlags) && !cmp) {
            return XJX_SPECTEXT_XMLTYPE;
        }
        return XJX_NORNALTEXT_XMLTYPE;
    }

    string number2string(Value &number)
    {
        stringstream ss;
        string buf;

        if (number.IsFloat()) {
            ss << number.GetFloat();
            ss >> buf;
        } else if (number.IsInt()) {
            ss << number.GetInt();
            ss >> buf;
        } else if (number.IsString()) {
            buf = string(number.GetString());
        }

        return buf;
    }

    string number2string(Value::ConstValueIterator &itrc)
    {
        stringstream ss;
        string buf;

        if (itrc->IsFloat()) {
            ss << itrc->GetFloat();
            ss >> buf;
        } else if (itrc->IsInt()) {
            ss << itrc->GetInt();
            ss >> buf;
        } else if (itrc->IsString()) {
            buf = string(itrc->GetString());
        }

        return buf;
    }

    vector<string> json2xml_parse_array_elem(const char *arrName, Value &arr, xml_node<> *parentNode)
    {
        vector<string> retlist;

        for (Value::ConstValueIterator itrc = arr.Begin(); itrc != arr.End(); ++itrc) {
            string elem;
            xml_node<> * newNode;

            if (itrc->IsObject()) {
                newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, arrName);
                Value *obj = const_cast<Value *>(itrc);
                elem = json2xml_traverse(*obj, newNode);
                if (elem.size() != 0)
                    retlist.push_back(elem);
            } else {
                elem = number2string(itrc);
                if (parse_xjx_objName(arrName) == XJX_SPECTEXT_XMLTYPE) {
                    newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, arrName+1, j2x_xmlDoc.allocate_string(elem.data()));
                }else{
                    newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, arrName, j2x_xmlDoc.allocate_string(elem.data()));
                }
            }
            parentNode->append_node(newNode);
        }

        return retlist;
    }

    void json2xml_parse_array(const char *objName, Value &arrObj, xml_node<> *parentNode)
    {
        xml_node<> *newNode;
        xml_node<> *currNode, *nextNode;
        xml_attribute<> *attrPos;

        vector<string> retlist;
        int i;

        retlist = json2xml_parse_array_elem(objName, arrObj[objName], parentNode);
        if (retlist.empty())
            return ;

        currNode = parentNode->first_node(objName);
        for(i = 0; i < (int)retlist.size(); i++) {
            string str = retlist.at(i);
            newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName, j2x_xmlDoc.allocate_string(str.data()));

            for (attrPos = currNode->first_attribute(); ; attrPos = attrPos->next_attribute()){
                newNode->append_attribute(j2x_xmlDoc.allocate_attribute(attrPos->name(),attrPos->value()));
                if (attrPos == currNode->last_attribute())
                    break;
            }
            parentNode->append_node(newNode);
            nextNode = currNode->next_sibling();
            parentNode->remove_node(currNode);
            currNode = nextNode;
        }
    }

    void json2xml_parse_object(const char *objName, xml_node<> *parentNode)
    {
        xml_node<> *newNode;

        int xjxXmlType = parse_xjx_objName(objName);
        switch (xjxXmlType) {
          case XJX_ATTR_XMLTYPE:
            parentNode->append_attribute(j2x_xmlDoc.allocate_attribute((objName+1), "null"));
            break;
          default:
            newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element,(objName+1));
            parentNode->append_node(newNode);
        }
    }

    string json2xml_parse_object(const char *objName, const char *objValue, xml_node<> *parentNode)
    {
        xml_node<> *newNode;
        string str;

        int xjxXmlType = parse_xjx_objName(objName);
        switch (xjxXmlType) {
          case XJX_ATTR_XMLTYPE:
            parentNode->append_attribute(j2x_xmlDoc.allocate_attribute((objName+1), j2x_xmlDoc.allocate_string(objValue)));
            break;
        case XJX_TEXT_XMLTYPE:
            newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element,(objName+1),  j2x_xmlDoc.allocate_string(objValue));
            parentNode->append_node(newNode);
            break;
        case XJX_SPECTEXT_XMLTYPE:
            str = string(objValue);
            break;
        case XJX_NORNALTEXT_XMLTYPE:
            newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName,  j2x_xmlDoc.allocate_string(objValue));
            parentNode->append_node(newNode);
        }

        return str;
    }

    void json2xml_parse_object(const char *objName, Value &parentobj, xml_node<> *parentNode)
    {
        xml_node<> *newNode, *newNode1;
        xml_attribute<> *attrPos;
        string str;

        newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName);

        str = json2xml_traverse(parentobj[objName], newNode);
        if (str.size() == 0) {
            parentNode->append_node(newNode);
        } else{
            newNode1 = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName, j2x_xmlDoc.allocate_string(str.data()));
            for (attrPos = newNode->first_attribute(); ; attrPos = attrPos->next_attribute()){
                newNode1->append_attribute(j2x_xmlDoc.allocate_attribute(attrPos->name(),attrPos->value()));
                if (attrPos == newNode->last_attribute())
                    break;
            }
            parentNode->append_node(newNode1);
        }

        return ;
    }

    void json2xml_parse_object(const char *objName)
    {
        xml_node<>* newNode, *newNode1;
        xml_attribute<> *attrPos;
        string str;

        newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName);

        str = json2xml_traverse(j2x_jsonDoc[objName], newNode);
        if (str.size() == 0) {
            j2x_xmlDoc.append_node(newNode);
        } else{
            newNode1 = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName, j2x_xmlDoc.allocate_string(str.data()));
            for (attrPos = newNode->first_attribute(); ; attrPos = attrPos->next_attribute()){
                cout << attrPos->name()  << ":" << attrPos->value() << endl;
                newNode1->append_attribute(j2x_xmlDoc.allocate_attribute(attrPos->name(),attrPos->value()));
                if (attrPos == newNode->last_attribute())
                    break;
            }
            j2x_xmlDoc.append_node(newNode1);
        }
    }

    string json2xml_traverse(Value &obj, xml_node<> *parentNode)
    {
        string ret;

        for (Value::ConstMemberIterator itr = obj.MemberBegin();itr != obj.MemberEnd(); ++itr){

            string str;

            const char *objName = itr->name.GetString();
            int type = itr->value.GetType();

            switch (type) {
            case kNullType:
                json2xml_parse_object(objName, parentNode);
                break;
            case kFalseType:
                str = json2xml_parse_object(objName, "false", parentNode);
                if (str.size() != 0)
                    ret = str;
                break;
            case kTrueType:
                str = json2xml_parse_object(objName, "true", parentNode);
                if (str.size() != 0)
                    ret = str;
                break;
            case kObjectType:
                json2xml_parse_object(objName, obj, parentNode);
                break;
            case kArrayType:
                json2xml_parse_array(objName, obj, parentNode);
                break;
            case kStringType:
                str = json2xml_parse_object(objName, itr->value.GetString(), parentNode);
                if (str.size() != 0)
                    ret = str;
                break;
            case kNumberType:
                string num = number2string(obj[objName]);
                str = json2xml_parse_object(objName, num.data(), parentNode);
                if (str.size() != 0)
                    ret = str;
                break;
            }
        }

        //   cout << "outside for, ret =" << ret << endl;
        return ret;
    }

    void json2xml_traverse(void)
    {
        xml_node<>* root = j2x_xmlDoc.allocate_node(rapidxml::node_pi, j2x_xmlDoc.allocate_string("xml version='1.0' encoding='utf-8'"));
        j2x_xmlDoc.append_node(root);

        for (Value::ConstMemberIterator itr = j2x_jsonDoc.MemberBegin();itr != j2x_jsonDoc.MemberEnd(); ++itr){
            xml_node<>* newNode;

            if (rootNodeArrFlags) //根节点的兄弟节点不能存在数组节点,否则xml2json不能等价转换
                break;

            const char *objName = itr->name.GetString();
            int type = itr->value.GetType();

            switch (type) {
            case kNullType:
                newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName);
                j2x_xmlDoc.append_node(newNode);
                break;
            case kFalseType:
                //cout << "False" << endl;
                newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName, "false");
                j2x_xmlDoc.append_node(newNode);
                break;
            case kTrueType:
                //cout << "True" << endl;
                newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName, "true");
                j2x_xmlDoc.append_node(newNode);
                break;
            case kObjectType:
                json2xml_parse_object(objName);
                break;
            case kArrayType:
                rootNodeArrFlags = 1;
                break;
            case kStringType:
                newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName, itr->value.GetString());
                j2x_xmlDoc.append_node(newNode);
                break;
            case kNumberType:
                string num = number2string(j2x_jsonDoc[objName]);
                newNode = j2x_xmlDoc.allocate_node(rapidxml::node_element, objName, j2x_xmlDoc.allocate_string(num.data()));
                j2x_xmlDoc.append_node(newNode);
                break;
            }
        }

        return ;
    }


    /*
     * xml2json function
     * clone https://github.com/Cheedoong/xml2json
     */
    static bool xml2json_has_digits_only(const char * input, bool *hasDecimal)
    {
        if (input == nullptr)
            return false;  // treat empty input as a string (probably will be an empty string)

        const char * runPtr = input;

        *hasDecimal = false;

        while (*runPtr != '\0')
        {
            if (*runPtr == '.')
            {
                if (!(*hasDecimal))
                    *hasDecimal = true;
                else
                    return false; // we found two dots - not a number
            }
            else if (isalpha(*runPtr))
            {
                return false;
            }
            runPtr++;
        }

        return true;
    }

    void xml2json_to_array_form(const char *name, rapidjson::Value &jsvalue, rapidjson::Value &jsvalue_chd, rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value jsvalue_target; // target to do some operation
        rapidjson::Value jn;             // this is a must, partially because of the latest version of rapidjson
        jn.SetString(name, allocator);
        jsvalue_target = jsvalue.FindMember(name)->value;
        if(jsvalue_target.IsArray())
        {
            jsvalue_target.PushBack(jsvalue_chd, allocator);
            jsvalue.RemoveMember(name);
            jsvalue.AddMember(jn, jsvalue_target, allocator);
        }
        else
        {
            rapidjson::Value jsvalue_array;
            //jsvalue_array = jsvalue_target;
            jsvalue_array.SetArray();
            jsvalue_array.PushBack(jsvalue_target, allocator);
            jsvalue_array.PushBack(jsvalue_chd, allocator);
            jsvalue.RemoveMember(name);
            jsvalue.AddMember(jn, jsvalue_array, allocator);
        }
    }

    void xml2json_add_attributes(rapidxml::xml_node<> *xmlnode, rapidjson::Value &jsvalue, rapidjson::Document::AllocatorType& allocator)
    {
        rapidxml::xml_attribute<> *myattr;
        for(myattr = xmlnode->first_attribute(); myattr; myattr = myattr->next_attribute())
        {
            rapidjson::Value jn, jv;
            jn.SetString((std::string(xml2json_attribute_name_prefix) + myattr->name()).c_str(), allocator);

            if (xml2json_numeric_support == false)
            {
                jv.SetString(myattr->value(), allocator);
            }
            else
            {
                bool hasDecimal;
                if (xml2json_has_digits_only(myattr->value(), &hasDecimal) == false)
                {
                    jv.SetString(myattr->value(), allocator);
                }
                else
                {
                    if (hasDecimal)
                    {
                        double value = std::strtod(myattr->value(),nullptr);
                        jv.SetDouble(value);
                    }
                    else
                    {
                        long int value = std::strtol(myattr->value(), nullptr, 0);
                        jv.SetInt(value);
                    }
                }
            }
            jsvalue.AddMember(jn, jv, allocator);
        }
    }

    void xml2json_traverse_node(rapidxml::xml_node<> *xmlnode, rapidjson::Value &jsvalue, rapidjson::Document::AllocatorType& allocator)
    {
        //cout << "this: " << xmlnode->type() << " name: " << xmlnode->name() << " value: " << xmlnode->value() << endl;
        rapidjson::Value jsvalue_chd;

        jsvalue.SetObject();
        jsvalue_chd.SetObject();
        rapidxml::xml_node<> *xmlnode_chd;

        // classified discussion:
        if((xmlnode->type() == rapidxml::node_data || xmlnode->type() == rapidxml::node_cdata) && xmlnode->value())
        {
            // case: pure_text
            jsvalue.SetString(xmlnode->value(), allocator);  // then addmember("#text" , jsvalue, allocator)
        }
        else if(xmlnode->type() == rapidxml::node_element)
        {
            if(xmlnode->first_attribute())
            {
                if(xmlnode->first_node() && xmlnode->first_node()->type() == rapidxml::node_data && count_children(xmlnode) == 1)
                {
                    // case: <e attr="xxx">text</e>
                    rapidjson::Value jn, jv;
                    jn.SetString(xml2json_text_additional_name, allocator);
                    jv.SetString(xmlnode->first_node()->value(), allocator);
                    jsvalue.AddMember(jn, jv, allocator);
                    xml2json_add_attributes(xmlnode, jsvalue, allocator);
                    return;
                }
                else
                {
                    // case: <e attr="xxx">...</e>
                    xml2json_add_attributes(xmlnode, jsvalue, allocator);
                }
            }
            else
            {
                if(!xmlnode->first_node())
                {
                    // case: <e />
                    jsvalue.SetNull();
                    return;
                }
                else if(xmlnode->first_node()->type() == rapidxml::node_data && count_children(xmlnode) == 1)
                {
                    // case: <e>text</e>
                    if (xml2json_numeric_support == false)
                    {
                        jsvalue.SetString(rapidjson::StringRef(xmlnode->first_node()->value()), allocator);
                    }
                    else
                    {
                        bool hasDecimal;
                        if (xml2json_has_digits_only(xmlnode->first_node()->value(), &hasDecimal) == false)
                        {
                            jsvalue.SetString(rapidjson::StringRef(xmlnode->first_node()->value()), allocator);
                        }
                        else
                        {
                            if (hasDecimal)
                            {
                                double value = std::strtod(xmlnode->first_node()->value(), nullptr);
                                jsvalue.SetDouble(value);
                            }
                            else
                            {
                                long int value = std::strtol(xmlnode->first_node()->value(), nullptr, 0);
                                jsvalue.SetInt(value);
                            }
                        }
                    }
                    return;
                }
            }
            if(xmlnode->first_node())
            {
                // case: complex else...
                std::map<std::string, int> name_count;
                for(xmlnode_chd = xmlnode->first_node(); xmlnode_chd; xmlnode_chd = xmlnode_chd->next_sibling())
                {
                    std::string current_name;
                    const char *name_ptr = NULL;
                    rapidjson::Value jn, jv;
                    if(xmlnode_chd->type() == rapidxml::node_data || xmlnode_chd->type() == rapidxml::node_cdata)
                    {
                        current_name = xml2json_text_additional_name;
                        name_count[current_name]++;
                        jv.SetString(xml2json_text_additional_name, allocator);
                        name_ptr = jv.GetString();
                    }
                    else if(xmlnode_chd->type() == rapidxml::node_element)
                    {
                        current_name = xmlnode_chd->name();
                        name_count[current_name]++;
                        name_ptr = xmlnode_chd->name();
                    }
                    xml2json_traverse_node(xmlnode_chd, jsvalue_chd, allocator);
                    if(name_count[current_name] > 1 && name_ptr)
                        xml2json_to_array_form(name_ptr, jsvalue, jsvalue_chd, allocator);
                    else
                    {
                        jn.SetString(name_ptr, allocator);
                        jsvalue.AddMember(jn, jsvalue_chd, allocator);
                    }
                }
            }
        }
        else
        {
            std::cerr << "err data!!" << std::endl;
        }
    }

public:
    /*
     * json2xml function
     */
    string json2xml_file(char *fileName)
    {
        char readBuffer[65536] = {};

        FILE* fp = fopen(fileName, "r");
        if (!fp)
            return "";

        FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        fclose(fp);

        j2x_jsonDoc.ParseStream(is);
        if (j2x_jsonDoc.HasParseError()) {
            ParseErrorCode errCode = j2x_jsonDoc.GetParseError();
            cout <<  "HasParseError ErrCode:" << errCode << " ErrOffset:" << j2x_jsonDoc.GetErrorOffset() << " ErrMessage:" << GetParseError_En(errCode) << endl;
            return "";
        }

        json2xml_traverse();

        string s;
        print(std::back_inserter(s), j2x_xmlDoc, 0);
        //print(std::cout, j2x_xmlDoc, 0);

        return s;
    }

    string json2xml(char *json)
    {
        j2x_jsonDoc.Parse(json);
        if (j2x_jsonDoc.HasParseError()) {
            ParseErrorCode errCode = j2x_jsonDoc.GetParseError();
            cout <<  "HasParseError ErrCode:" << errCode << " ErrOffset:" << j2x_jsonDoc.GetErrorOffset() << " ErrMessage:" << GetParseError_En(errCode) << endl;
            return "";
        }

        json2xml_traverse();

        string s;
        print(std::back_inserter(s), j2x_xmlDoc, 0);

        //print(std::cout, j2x_xmlDoc, 0);

        return s;
    }

    /*
     * xml2json function
     * clone https://github.com/Cheedoong/xml2json
     */
    string xml2json(const char *xml_str)
    {
        x2j_jsonDoc.SetObject();
        Document::AllocatorType& allocator = x2j_jsonDoc.GetAllocator();

        x2j_xmldoc.parse<0> (const_cast<char *>(xml_str));
        xml_node<> *xmlnode_chd;

        for(xmlnode_chd = x2j_xmldoc.first_node(); xmlnode_chd; xmlnode_chd = xmlnode_chd->next_sibling())
        {
            rapidjson::Value jsvalue_chd;
            jsvalue_chd.SetObject();
            xml2json_traverse_node(xmlnode_chd, jsvalue_chd, allocator);
            x2j_jsonDoc.AddMember(rapidjson::StringRef(xmlnode_chd->name()), jsvalue_chd, allocator);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        x2j_jsonDoc.Accept(writer);

        return buffer.GetString();
    }

    string xml2json_file(const char *xml_file)
    {
        file<> fdoc(xml_file);
        return xml2json(fdoc.data());
    }
};
#endif
