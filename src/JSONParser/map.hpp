#ifndef MAP_H
#define MAP_H

#include <string.h>

template<typename T>
class Map {
public:
    i64 size;
    char** hashTable;
    T* dataTable;

    Map() {}

    Map(i64 _size) :
        size(_size),
        hashTable((char**)malloc(_size * sizeof(char*))),
        dataTable((T*)malloc(_size * sizeof(T))) {}
    
    i64 getIndex(const char* key) {
        for (i64 i = 0; i < size; i++) {
            if (strcmp(key, hashTable[i]) == 0) {
                return i;
            }
        }
        return -1;
    }

    void set(const char* key, T data) {
        i64 index = getIndex(key);
        if (index == -1) {
            size++;

            hashTable = (char**)realloc(hashTable, size * sizeof(char*));
            hashTable[size - 1] = (char*)malloc((strlen(key) + 1) * sizeof(char));
            strcpy(hashTable[size - 1], key);
            hashTable[size - 1][strlen(key)] = 0;

            dataTable = (T*)realloc((T*)dataTable, size * sizeof(T));
            dataTable[size - 1] = data;
            
            return;
        }

        dataTable[index] = data;
    }

    void freeMap() {
        for (i64 i = 0; i < size; i++) {
            free(hashTable[i]);
            free(dataTable[i]);
        }
        free(hashTable);
        free(dataTable);
    }
};

#endif