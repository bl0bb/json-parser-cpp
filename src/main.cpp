#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "JSONParser/json_parser.hpp"

void addStringInJson(std::string& jsonString, std::string str, i64 depth) {
    std::string depthStr = "";
    for (i64 i = 0; i < depth * 2; i++) {
        depthStr += ' ';
    }
    for (i64 i = 0; i < str.size(); i++) {
        char ch = str[i];
        if (ch == '\n') {
            str.insert(i + 1, depthStr);
        }
    }
    jsonString += str;
}

std::string traverseJson(JSONValue* json, i64 depth) {
    std::string jsonString = "";

    if (json->type == u8(JSONType::NONE)) {
        jsonString += "null";
    } else if (json->type == u8(JSONType::NUMBER)) {
        jsonString += std::to_string(*(f64*)json->value);
    } else if (json->type == u8(JSONType::STRING)) {
        jsonString += "\"";
        addStringInJson(jsonString, (char*)json->value, depth);
        jsonString += "\"";
    } else if (json->type == u8(JSONType::ARRAY)) {
        addStringInJson(jsonString, "[\n", depth + 1);
        std::vector<JSONValue*>* arr = (std::vector<JSONValue*>*)json->value;

        for (i64 i = 0; i < arr->size(); i++) {
            jsonString += traverseJson((*arr)[i], depth + 1);
            if (i < arr->size() - 1) {
                jsonString += ",";
                addStringInJson(jsonString, "\n", depth + 1);
            } else {
                addStringInJson(jsonString, "\n", depth);
            }
        }

        jsonString += "]";
    } else if (json->type == u8(JSONType::OBJECT)) {
        addStringInJson(jsonString, "{\n", depth + 1);
        Map<JSONValue*>* arr = (Map<JSONValue*>*)json->value;

        for (i64 i = 0; i < arr->size; i++) {
            jsonString += "\"" + std::string(arr->hashTable[i]) + "\": ";
            jsonString += traverseJson(arr->dataTable[i], depth + 1);
            if (i < arr->size - 1) {
                jsonString += ",";
                addStringInJson(jsonString, "\n", depth + 1);
            } else {
                addStringInJson(jsonString, "\n", depth);
            }
        }
        
        jsonString += "}";
    } else if (json->type == u8(JSONType::BOOL)) {
        u8 value = *(u8*)json->value;
        if (value) {
            jsonString += "true";
        } else {
            jsonString += "false";
        }
    } else if (json->type == u8(JSONType::UNKNOWN)) {
        std::cout << "ERROR: UNKNOWN TYPE\n";
    } else {
        std::cout << "ERROR: INVALID TYPE\n";
    }

    return jsonString;
}

int runTest(int testN) {
    std::ifstream file("tests/test" + std::to_string(testN + 1) + ".json");  // Replace with your file name
    std::string jsonStr;

    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        jsonStr = buffer.str();
        file.close();
    } else {
        std::cerr << "Unable to open file" << std::endl;
    }

    i64 i = 0;
    JSONValue* json;
    try {
        json = JSONParser::parse(jsonStr.c_str(), &i);
    } catch (std::runtime_error* e) {
        std::cout << "TEST FAILED: " << e->what() << "\n";
    }

    std::string formattedString = traverseJson(json, 0);
    std::cout << "FORMATTED:\n" + formattedString + "\n";

    return 0;
}

int runTests() {
    for (int i = 0; i < 47; i++) {
        int code = runTest(i);
        if (code != 0) {
            std::cout << "FAILED TEST " + std::to_string(i + 1) + " " + std::to_string(code) + "\n";
            return code;
        }
        std::cout << "PASSED TEST " + std::to_string(i + 1) + "\n";
    }
    return 0;
}

int main(int const argc, char const*const* argv) {
    int code = runTests();
    if (code != 0) {
        std::cout << "FAILED " + std::to_string(code) + "\n";
    }
    std::cout << "SUCCESS\n";
    return 0;
}