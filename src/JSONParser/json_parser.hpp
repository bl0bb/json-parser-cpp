#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>

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

0 = null -> value not present
0 = number -> i64
1 = string -> char*
2 = array -> std::vector<JSONValue*>
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

    void freeSelf() {
        if (type == u8(JSONType::UNKNOWN) || type == u8(JSONType::NONE)) {
            return;
        }
        
        free(value);
    }

    void freeAll() {
        if (type == u8(JSONType::UNKNOWN) || type == u8(JSONType::NONE)) {
            return;
        }

        if (type == u8(JSONType::ARRAY)) {
            std::vector<JSONValue*>* arr = (std::vector<JSONValue*>*)value;
            for (u8 i = 0; i < arr->size(); i++) {
                (*arr)[i]->freeAll();
            }
            arr->clear();
        } else if (type == u8(JSONType::OBJECT)) {
            Map<JSONValue*>* map = (Map<JSONValue*>*)value;
            for (i64 i = 0; i < map->size; i++) {
                map->dataTable[i]->freeAll();
            }
            map->freeMap();
        }

        free(value);
    }
};

class JSONParser {
public:
    static bool isWhitespace(char ch) {
        return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
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

        i64 backslashCount = 0;
        
        char* myStr = (char*)malloc(0);
        while (true) {
            char checkCh = jsonString[++(*i)];
            if (!checkCh) {
                break;
            }
            if (checkCh == '\\') {
                if (backslashCount) {
                    backslashCount++;
                } else {
                    backslashCount = 1;
                }
                continue;
            }
            if (backslashCount) {
                len += backslashCount;
                myStr = (char*)realloc(myStr, (len + 1) * sizeof(char*));
                for (i64 j = 0; j < backslashCount; j++) {
                    myStr[len - 1 - j] = '\\';
                }

                if (checkCh == '"') {
                    if ((backslashCount & 1) == 0) {
                        break;
                    }
                }

                backslashCount = 0;

                len++;
                myStr[len - 1] = checkCh;
                myStr[len] = 0;
            } else {
                if (checkCh == '"') {
                    break;
                }

                len++;
                myStr = (char*)realloc(myStr, (len + 1) * sizeof(char*));
                myStr[len - 1] = checkCh;
                myStr[len] = 0;
            }
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
            f64 num = 0;
            bool hasDecimal = false;
            i64 decimalOff;
            while (true) {
                char checkCh = jsonString[i];
                if (!checkCh) {
                    break;
                }
                if (checkCh == '.') {
                    if (hasDecimal) {
                        throw new std::runtime_error("Multiple decimal points in number");
                    }
                    hasDecimal = true;
                    decimalOff = 0;
                    i++;
                    continue;
                } else if (checkCh < '0' || checkCh > '9') {
                    break;
                }
                if (!hasDecimal) {
                    num *= 10;
                    num += checkCh - '0';
                } else {
                    decimalOff++;
                    num += f64(checkCh - '0') / std::pow(10, decimalOff);
                }
                i++;
            }
            skipWhitespace(jsonString, &i);

            result->type = u8(JSONType::NUMBER);
            result->value = malloc(sizeof(f64));
            *(f64*)result->value = num;
        } else if (ch == '"') {
            char* str;
            i64 len = parseString(jsonString, &i, &str);
            skipWhitespace(jsonString, &i);

            result->type = u8(JSONType::STRING);
            result->value = str;
        } else if (ch == '[') {
            i++;
            skipWhitespace(jsonString, &i);

            result->type = u8(JSONType::ARRAY);
            
            result->value = malloc(sizeof(std::vector<JSONValue*>));
            new (result->value) std::vector<JSONValue*>();

            while (true) {
                skipWhitespace(jsonString, &i);

                char earlyEndCh = jsonString[i];
                if (earlyEndCh == ']') {
                    i++;
                    break;
                }

                JSONValue* newValue = parse(jsonString, &i);
                skipWhitespace(jsonString, &i);

                ((std::vector<JSONValue*>*)result->value)->push_back(newValue);

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
                throw new std::runtime_error("Expected symbol near end of array");
            }
        } else if (ch == '{') {
            i++;
            skipWhitespace(jsonString, &i);

            result->type = u8(JSONType::OBJECT);

            result->value = malloc(sizeof(Map<JSONValue*>));
            new (result->value) Map<JSONValue*>(0);


            while (true) {
                skipWhitespace(jsonString, &i);

                char checkCh = jsonString[i];
                if (!checkCh) {
                    break;
                }
                if (checkCh == '}') {
                    i++;
                    break;
                }
                if (checkCh != '"') {
                    throw new std::runtime_error("Missing double quotes for key in object");
                }

                char* keyStr;
                i64 keyLen = parseString(jsonString, &i, &keyStr);
                skipWhitespace(jsonString, &i);

                char colonCh = jsonString[i++];
                if (colonCh != ':') {
                    throw new std::runtime_error("Missing colon after key in object");
                }
                skipWhitespace(jsonString, &i);

                JSONValue* newValue = parse(jsonString, &i);
                skipWhitespace(jsonString, &i);

                ((Map<JSONValue*>*)result->value)->set(keyStr, newValue);
                free(keyStr);

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
                throw new std::runtime_error("Expected symbol near end of object");
            }
        } else {
            skipWhitespace(jsonString, &i);

            JSONKeyword keyword = parseKeyword(jsonString, &i);
            skipWhitespace(jsonString, &i);

            if (keyword == JSONKeyword::UNKNOWN) {
                result->type = u8(JSONType::UNKNOWN);
            } else if (keyword == JSONKeyword::NONE) {
                result->type = u8(JSONType::NONE);
            } else if (keyword == JSONKeyword::FALSE || keyword == JSONKeyword::TRUE) {
                result->type = u8(JSONType::BOOL);
                result->value = malloc(sizeof(u8));
                *(u8*)result->value = u8(keyword) - u8(JSONKeyword::FALSE);
            } else {
                throw new std::runtime_error("Unknown keyword");
            }
        }

        *startI = i;

        return result;
    }
};

#endif