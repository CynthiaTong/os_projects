#include <string>

#ifndef STRUCTS_H
#define STRUCTS_H

struct Record {
    int id;
    std::string last;
    std::string first;
    int age;
    int year;
    double gpa;
    int numCourses;
};

struct Node
{
    int key;
    Record* recptr;
    struct Node** forward;

    Node(int maxLevel, int k, Node* forwardTarget=nullptr, Record* rec=nullptr)
    {
        key = k;
        forward = new Node*[maxLevel]();
        recptr = rec;
        for (int i = 0; i < maxLevel; i++)
            forward[i] = forwardTarget;
    }

    ~Node()
    {
        delete recptr;
        delete[] forward;
    }
};

#endif
