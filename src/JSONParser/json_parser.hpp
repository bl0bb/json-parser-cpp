#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdlib.h>
#include <iostream>
#include <vector>

#include "types.hpp"
#include "array.hpp"
#include "map.hpp"

const char* jsonKeywords[] = {
    "null",
    "false",
    "true",
};

enum class JSONKeyword : i8 {
    UNKNOWN = -1,
    NONE = 0, // cant use NULL since its a keyword in C/C++
    FALSE = 1,
    TRUE = 2,
};

/*
types

0 = null -> u8 = 0
0 = number -> i64
1 = string -> char*
2 = array -> JSONValue*
3 = object -> Map<JSONValue*>
4 = bool -> bool

*/

enum class JSONType : i8 {
    UNKNOWN = -1,
    NONE = 0,
    NUMBER = 1,
    STRING = 2,
    ARRAY = 3,
    OBJECT = 4,
    BOOL = 5,
};

struct JSONValue {
    u8 type;
    void* value;
};

class JSONParser {
public:
    static bool isWhitespace(char ch) {
        return ch == ' ' || ch == '\n' || ch == '\t';
    }

    static void skipWhitespace(const char* jsonString, i64* i) {
        while (isWhitespace(jsonString[*i])) {
            (*i)++;
        }
    }

    static JSONKeyword parseKeyword(const char* jsonString, i64* i) {
        for (i64 j = 0; j < array_size(jsonKeywords); j++) {
            const char* keyword = jsonKeywords[j];

            i64 k = 0;
            while (true) {
                char checkCh = jsonString[*i + k];
                char keywordCh = keyword[k];
                if (!keywordCh) {
                    *i += k;
                    return JSONKeyword(j);
                }
                if (checkCh != keywordCh) {
                    break;
                }
                k++;
            }
        }

        return JSONKeyword::UNKNOWN;
    }

    static i64 parseString(const char* jsonString, i64* i, char** str) {
        i64 len = 0;

        char* myStr = (char*)malloc(0);
        while (true) {
            char checkCh = jsonString[++(*i)];
            if (!checkCh) {
                break;
            }
            if (checkCh == '"') {
                break;
            }
            len++;
            myStr = (char*)realloc(myStr, (len + 1) * sizeof(char*));
            myStr[len - 1] = checkCh;
        }
        myStr[len] = 0;
        *str = myStr;
        (*i)++;

        return len;
    }

    static JSONValue* parse(const char* jsonString, i64* startI) {
        JSONValue* result = (JSONValue*)malloc(sizeof(JSONValue));

        i64 i = *startI;
        skipWhitespace(jsonString, &i);

        char ch = jsonString[i];
        if (ch >= '0' && ch <= '9') {
            i64 num = 0;
            while (true) {
                char checkCh = jsonString[i++];
                if (!checkCh) {
                    break;
                }
                if (checkCh < '0' || checkCh > '9') {
                    break;
                }
                num *= 10;
                num += checkCh - '0';
            }
            result->type = u8(JSONType::NUMBER);
            result->value = malloc(sizeof(i64));
            *(i64*)result->value = num;
        } else if (ch == '"') {
            char* str;
            i64 len = parseString(jsonString, &i, &str);
            result->type = u8(JSONType::STRING);
            result->value = str;
        } else if (ch == '[') {
            i++;
            skipWhitespace(jsonString, &i);
            result->type = u8(JSONType::ARRAY);

            result->value = new std::vector<JSONValue*>();
            while (true) {
                skipWhitespace(jsonString, &i);
                JSONValue* newValue = parse(jsonString, &i);
                std::cout << "NEW VALUE: " << std::to_string(newValue->type) << "\n";
                ((std::vector<JSONValue*>*)result->value)->push_back(newValue);
                skipWhitespace(jsonString, &i);
                char checkCh = jsonString[i];
                if (!checkCh) {
                    break;
                }
                if (checkCh == ']') {
                    i++;
                    skipWhitespace(jsonString, &i);
                    break;
                }
                if (checkCh == ',') {
                    i++;
                    skipWhitespace(jsonString, &i);
                    continue;
                }
                std::cout << "WOMP WOMP 1 " + std::to_string(checkCh) + "\n";
            }
        } else if (ch == '{') {
            i++;
            skipWhitespace(jsonString, &i);
            result->type = u8(JSONType::OBJECT);
            
            result->value = new Map<JSONValue*>(0);
            while (true) {
                skipWhitespace(jsonString, &i);
                char checkCh = jsonString[i];
                if (checkCh != '"') {
                    std::cout << "WOMP WOMP 3 " << std::to_string(i) << " " << std::to_string(checkCh) << "\n";
                    throw new std::runtime_error("whoopsieessssss");
                }
                if (checkCh == '}') {
                    std::cout << "WOMP WOMP 3 " << std::to_string(i) << " " << std::to_string(checkCh) << "\n";
                    throw new std::runtime_error("whoopsieessssss");
                }

                char* keyStr;
                i64 keyLen = parseString(jsonString, &i, &keyStr);

                char colonCh = jsonString[i++];
                if (colonCh != ':') {
                    std::cout << "WOMP WOMP 5\n";
                }
                skipWhitespace(jsonString, &i);

                std::cout << "mongodb " << keyStr << " " << std::to_string(i) << "\n";
                
                JSONValue* newValue = parse(jsonString, &i);
                
                ((Map<JSONValue*>*)result->value)->set(keyStr, newValue);

                char endCh = jsonString[i];
                if (endCh == '}') {
                    i++;
                    skipWhitespace(jsonString, &i);
                    break;
                }
                if (endCh == ',') {
                    i++;
                    skipWhitespace(jsonString, &i);
                    continue;
                }
                std::cout << "WOMP WOMP 4 " << std::to_string(endCh) << "\n";
            }
        } else {
            JSONKeyword keyword = parseKeyword(jsonString, &i);
            if (keyword == JSONKeyword::UNKNOWN) {
                result->type = u8(JSONType::UNKNOWN);
            } else if (keyword == JSONKeyword::NONE) {
                result->type = u8(JSONType::NONE);
            } else if (keyword == JSONKeyword::FALSE || keyword == JSONKeyword::TRUE) {
                result->type = u8(JSONType::BOOL);
                result->value = malloc(sizeof(u8));
                *(u8*)result->value = u8(keyword) - u8(JSONKeyword::FALSE);
            } else {
                std::cout << "WOMP WOMP 2\n";
            }
        }

        *startI = i;

        return result;
    }
};

#endif