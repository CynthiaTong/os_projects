#include <sstream>

#include "myapp.h"
using namespace std;

MyApp::MyApp(int maxL, int maxV)
{
    lst = new SkipList(maxL, maxV);
    maxLevel = maxL;
    maxValue = maxV;
}

int MyApp::ins(vector<string>& params)
{
    int inserted;
    try {
        int key = stoi(params[0]);
        // make a new student record with params
        Record* r = new Record();
        r->id = key;
        r->last = params[1];
        r->first = params[2];
        r->age = stoi(params[3]);
        r->year = stoi(params[4]);
        r->gpa = stod(params[5]);
        r->numCourses = stoi(params[6]);

        inserted = lst->insert(key, r);
    }
    catch (...)
    {
        return 1;
    }
    return inserted;
}

Record* MyApp::find(int studID, int* count)
{
    Node* n = lst->find(studID, count);
    if (n)
        return n->recptr;
    else
        return nullptr;
}

string MyApp::range(int studID1, int studID2)
{

    // safety check for correct range
    if (studID1 <= 0)   studID1 = 1;
    if (studID1 >= maxValue)    studID1 = maxValue-1;
    if (studID2 <= 0)   studID2 = 1;
    if (studID2 >= maxValue)    studID2 = maxValue-1;

    // n1 is the smallest node larger than studID1
    Node* n1 = lst->search(studID1)->forward[0];
    // n2 is the largest node smaller than studID2
    Node* n2 = lst->search(studID2);

    // if no nodes exist in this range, return empty string
    if (n1 == n2->forward[0])
        return "";

    // if n2->forward[0] is actually studID2, point n2 to that
    if (n2->forward[0] && n2->forward[0]->key == studID2)
        n2 = n2->forward[0];

    Record* r;
    stringstream ss;
    ss << "ID, Last, First, Age, Year, GPA, #Courses\n";

    Node* stop = n2->forward[0];
    while (n1 != stop)
    {
        r = n1->recptr;
        ss << r->id << ", " << r->last << ", " << r->first << ", " << r->age << ", "
            << r->year << ", " << r->gpa << ", " << r->numCourses << endl;

        n1 = n1->forward[0];
    }

    return ss.str();
}

double MyApp::gpa(int studID1, int studID2)
{
    if (studID2 == -1)
    {
        Node* n = lst->find(studID1);
        if (n)
            return n->recptr->gpa;
        else
            return -1;
    }

    // safety check for correct range
    if (studID1 <= 0)   studID1 = 1;
    if (studID1 >= maxValue)    studID1 = maxValue-1;
    if (studID2 <= 0)   studID2 = 1;
    if (studID2 >= maxValue)    studID2 = maxValue-1;

    // n1 is the smallest node larger than studID1
    Node* n1 = lst->search(studID1)->forward[0];
    // n2 is the largest node smaller than studID2
    Node* n2 = lst->search(studID2);

    // if n2->forward[0] is actually studID2, point n2 to that
    if (n2->forward[0] && n2->forward[0]->key == studID2)
        n2 = n2->forward[0];

    double sum = 0;
    int count = 0;
    Node* stop = n2->forward[0];
    while (n1 != stop)
    {
        sum += n1->recptr->gpa;
        n1 = n1->forward[0];
        count++;
    }
    // if no nodes found between the given range, return -1
    if (count == 0)
        return -1;
    return sum/count;
}

int MyApp::del(int studID)
{
    return lst->remove(studID);
}

string MyApp::print()
{
   return lst->tostring();
}

void MyApp::exit()
{
    lst->removeAll();
    delete lst;
}

