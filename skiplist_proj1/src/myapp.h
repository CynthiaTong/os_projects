#include <vector>

#include "skiplist.h"

#ifndef MYAPP_H
#define MYAPP_H

class MyApp {
    SkipList* lst;
    int maxLevel;
    int maxValue;
public:
    MyApp(int, int);
    int ins(std::vector<std::string>& params);
    Record* find(int studID, int* count=nullptr);
    std::string range(int studID1, int studID2);
    double gpa(int studID1, int studID2=-1);
    int del(int studID);
    std::string print();
    void exit();
};

#endif
