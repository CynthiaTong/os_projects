#include "structs.h"

#ifndef SKIPLIST_H
#define SKIPLIST_H

class SkipList {
    int maxLevel;
    int maxValue;
    Node* head;
    Node* tail;
    int randomLevel();

public:
    SkipList(int maxL, int maxV);
    Node* find(int target, int* numComparisons=nullptr);
    Node* search(int target, int* numComparisons=nullptr);
    int insert(int key, Record* rec);
    int remove(int key);
    void removeAll();
    std::string tostring();
};

#endif
