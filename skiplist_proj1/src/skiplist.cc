#include <ctime>
#include <sstream>

#include "skiplist.h"
using namespace std;

SkipList::SkipList(int maxL, int maxV)
{
    tail = new Node(maxL, maxV);
    head = new Node(maxL, 0, tail);
    maxLevel = maxL;
    maxValue = maxV;

    // seed random number generator
    srand(time(0));
}

int SkipList::randomLevel()
{
    int level = 0;
    // random 0 or 1 (half probability of increasing each one level)
    while ((rand() & 1) && level < maxLevel-1)
        level++;
    return level;
}

/*
 * Wrapper method for the search method below.
 * returns nullptr if target is not found.
 * Used by the find/sfind commands in myapp.
 */
Node* SkipList::find(int target, int* numComparisons)
{
    Node* n = search(target, numComparisons);
    n = n->forward[0];
    if (n->key == target)
        return n;
    else
        return nullptr;
}

/*
 * Search for the target ID, returns either the node with that ID,
 * or the largest node whose ID is smaller than target.
 * Used by the range/gpa commands in myapp.
 */
Node* SkipList::search(int target, int* numComparisons)
{
    Node* n = head;
    for (int i = maxLevel-1; i >= 0; i--)
    {
        while(n->forward[i]->key < target) {
            n = n->forward[i];
            if (numComparisons) (*numComparisons)++;
        }
        // also count that one comparisons where n->forward[i]->key >= target
        if (numComparisons) (*numComparisons)++;
    }
    return n;
}

int SkipList::insert(int key, Record* rec)
{
    // safety check that key is in correct range
    if (key <= 0 || key >= maxValue)
        return 1;

    Node* n = head;
    Node** update = new Node*[maxLevel]();

    for (int i = maxLevel-1; i >= 0; i--)
    {
        while (n->forward[i]->key < key)
            n = n->forward[i];
        // populate the update array with pointers to nodes
        update[i] = n;
    }
    n = n->forward[0];

    if (n->key == key)
        n->recptr = rec;
    else
    {
        int lvl = randomLevel();
        Node* x = new Node(maxLevel, key, tail, rec);
        // update forward pointers of x and previous nodes
        for (int i = 0; i <= lvl; i++)
        {
            x->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = x;
        }
    }

    delete[] update;
    return 0;
}

int SkipList::remove(int key)
{
    // safety check that key is in correct range
    if (key <= 0 || key >= maxValue)
        return 1;

    Node* n = head;
    Node** update = new Node*[maxLevel]();

    for (int i = maxLevel-1; i >= 0; i--)
    {
        while (n->forward[i]->key < key)
            n = n->forward[i];
        // populate the update array with pointers to nodes
        update[i] = n;
    }
    n = n->forward[0];

    // if not found, return
    if (n->key != key)
        return 1;

    for (int i = 0; i < maxLevel; i++)
    {
        if (update[i]->forward[i] != n)
            break;
        update[i]->forward[i] = n->forward[i];
    }

    delete[] update;
    delete n;

    return 0;
}

void SkipList::removeAll()
{
    Node* n = head;
    Node* next;
    while (n)
    {
        next = n->forward[0];
        delete n;
        // iterate to next node
        n = next;
    }
}

string SkipList::tostring()
{
    stringstream ss;

    if (head->forward[0] == tail)
    {
        ss << "No existing records. Insert first!" << endl;
    }
    else
    {
        ss << "ID, Last, First, Age, Year, GPA, #courses\n";

        Node* n = head->forward[0];
        Record* r;
        while (n->forward[0] != nullptr)
        {
            r = n->recptr;
            ss << r->id << ", " << r->last << ", " << r->first << ", " << r->age << ", "
               << r->year << ", " << r->gpa << ", " << r->numCourses << endl;

            n = n->forward[0];
        }
    }

    return ss.str();
}

