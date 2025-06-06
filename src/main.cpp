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
}

std::string traverseJson(JSONValue* json, i64 depth) {
    std::string jsonString = "";

    if (json->type == u8(JSONType::NONE)) {
        addStringInJson(jsonString, "null", depth);
    } else if (json->type == u8(JSONType::NUMBER)) {
        addStringInJson(jsonString, std::to_string(*(i64*)json->value), depth);
    } else if (json->type == u8(JSONType::STRING)) {
        addStringInJson(jsonString, "\"", depth);
        addStringInJson(jsonString, (char*)json->value, depth);
        addStringInJson(jsonString, "\"", depth);
    } else if (json->type == u8(JSONType::ARRAY)) {
        jsonString += "[\n";
        std::vector<JSONValue*>* arr = (std::vector<JSONValue*>*)json->value;

        for (i64 i = 0; i < arr->size(); i++) {
            addStringInJson(jsonString, traverseJson((*arr)[i], depth + 1), depth + 1);
            addStringInJson(jsonString, ",\n", depth + 1);
        }

        jsonString += "]";
    } else if (json->type == u8(JSONType::OBJECT)) {
        jsonString += "{\n";
        Map<JSONValue*>* arr = (Map<JSONValue*>*)json->value;

        for (i64 i = 0; i < arr->size; i++) {
            addStringInJson(jsonString, "\"" + std::string(arr->hashTable[i]) + "\": ", depth + 1);
            addStringInJson(jsonString, traverseJson(arr->dataTable[i], depth + 1), depth + 1);
            addStringInJson(jsonString, ",\n", depth + 1);
        }
        
        jsonString += "}";
    } else if (json->type == u8(JSONType::BOOL)) {
        u8 value = *(u8*)json->value;
        if (value) {
            addStringInJson(jsonString, "true", depth);
        } else {
            addStringInJson(jsonString, "false", depth);
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
    JSONValue* json = JSONParser::parse(jsonStr.c_str(), &i);

    std::string formattedString = traverseJson(json, 0);
    std::cout << "FORMATTED:\n" + formattedString + "\n";

    return 0;
}

int runTests() {
    for (int i = 0; i < 5; i++) {
        int code = runTest(i);
        if (code != 0) {
            std::cout << "FAILED TEST " + std::to_string(i) + " " + std::to_string(code) + "\n";
            return code;
        }
        std::cout << "PASSED TEST " + std::to_string(i) + "\n";
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