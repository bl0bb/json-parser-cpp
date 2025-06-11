#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "JSONParser/json_parser.hpp"

constexpr u32 TEST_COUNT = 67;

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
        jsonString += "[";
        std::vector<JSONValue*>* arr = (std::vector<JSONValue*>*)json->value;

        if (arr->size() > 0) {
            addStringInJson(jsonString, "\n", depth + 1);
        }

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
        jsonString += "{";
        Map<JSONValue*>* map = (Map<JSONValue*>*)json->value;

        if (map->size > 0) {
            addStringInJson(jsonString, "\n", depth + 1);
        }

        for (i64 i = 0; i < map->size; i++) {
            jsonString += "\"" + std::string(map->hashTable[i]) + "\": ";
            jsonString += traverseJson(map->dataTable[i], depth + 1);
            if (i < map->size - 1) {
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

bool runTest(int testN) {
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
    bool didFail;
    try {
        json = JSONParser::parse(jsonStr.c_str(), &i);
        didFail = false;
    } catch (std::runtime_error* e) {
        std::cout << "TEST FAILED: " << e->what() << "\n";
        didFail = true;
    }

    std::string formattedString = traverseJson(json, 0);
    std::cout << "FORMATTED:\n" + formattedString + "\n";

    json->freeAll();

    return didFail;
}

int runTests() {
    int passCount = 0;
    for (int i = 0; i < TEST_COUNT; i++) {
        bool testCode = runTest(i);
        if (testCode == 1) {
            std::cout << "FAILED TEST " + std::to_string(i + 1) + "\n";
            continue;
        }
        passCount++;
        std::cout << "PASSED TEST " + std::to_string(i + 1) + "\n";
    }
    return passCount;
}

int main(int const argc, char const*const* argv) {
    int passCount = runTests();
    std::cout << "PASSED: " << passCount << "/" << TEST_COUNT << "\n";
    return 0;
}