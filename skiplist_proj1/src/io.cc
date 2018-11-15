#include "io.h"
using namespace std;

int handleIns(vector<string>& inputs, MyApp* app)
{
    if (app->ins(inputs) == 1)
    {
        cerr << "Failed to insert record for student ID: " << inputs[0]  << endl;
        return 1;
    }
    return 0;
}

int handleFind(string action, string id, MyApp* app)
{
    int studID;
    try {
        studID = stod(id);
    }
    catch (...)
    {
        cerr << "Invalid (s)find argument: student ID cannot be: " << id << endl;
        return 1;
    }

    // handle either find or sfind
    Record* r;
    int steps = 0;
    if (action == "find")
        r = app->find(studID);
    else
        r = app->find(studID, &steps);

    if (!r)
    {
        cerr << "Failed to find record for student ID: " << studID << endl;
        return 1;
    }
    else
    {
        cout << "Student record: ";
        cout << r->id << ", " << r->last << ", " << r->first << ", " << r->age << ", "
             << r->year << ", " << r->gpa << ", " << r->numCourses << endl;
        if (action == "sfind")
            cout << "Number of comparisons: " << steps << endl;
    }
    return 0;
}

int handleRange(string id1, string id2, MyApp* app)
{
    int studID1, studID2;
    try {
        studID1 = stod(id1);
        studID2 = stod(id2);
    }
    catch (...)
    {
        cerr << "Invalid range arguments: student IDs cannot be: " << id1 << " and " << id2 << endl;
        return 1;
    }

    // swap the two IDs if in wrong order
    if (studID1 > studID2)
    {
        int tmp = studID1;
        studID1 = studID2;
        studID2 = tmp;
    }

    string output = app->range(studID1, studID2);
    if (output == "")
    {
        cerr << "Records not found for ID range [" << studID1 << ", " << studID2 << "]" << endl;
        return 1;
    }
    else
        cout << output;
    return 0;
}

int handleGpa(string id1, string id2, MyApp* app)
{
    int studID1, studID2;
    bool computeAverage = id2 != "";

    try {
        studID1 = stod(id1);
        if (computeAverage)
            studID2 = stod(id2);
    }
    catch (...)
    {
        cerr << "Invalid gpa argument(s): student ID(s) cannot be: " << id1;
        if (computeAverage) cerr << " and " << id2;
        cerr << endl;
        return 1;
    }

    // swap the two IDs if in wrong order
    if (computeAverage && studID1 > studID2)
    {
        int tmp = studID1;
        studID1 = studID2;
        studID2 = tmp;
    }

    double averageGPA;
    if (computeAverage)
        averageGPA = app->gpa(studID1, studID2);
    else
        averageGPA = app->gpa(studID1);

    if (averageGPA == -1)
    {
        if (computeAverage)
            cerr << "GPA records not found in ID range [" << studID1 << ", " << studID2 << "]" << endl;
        else
            cerr << "GPA record not found for ID: " << studID1 << endl;
        return 1;
    }
    else
    {
        if (computeAverage)
            cout << "Average GPA in ID range [" << studID1 << ", " << studID2 << "]: " << averageGPA << endl;
        else
            cout << "GPA record for ID " << studID1 << ": "  << averageGPA << endl;
    }
    return 0;
}

int handleDel(string id, MyApp* app)
{
    int studID;
    try {
        studID = stod(id);
    }
    catch (...)
    {
        cerr << "Invalid del argument: student ID cannot be: " << id << endl;
        return 1;
    }

    if (app->del(studID) == 1)
    {
        cerr << "Failed to delete record for student ID: " << id  << endl;
        return 1;
    }
    return 0;
}

int handlePrint(MyApp* app)
{
    try {
        cout <<  app->print();
    }
    catch(...)
    {
        cerr << "Failed to print records." << endl;
        return 1;
    }
    return 0;
}

int handleExit(MyApp* app)
{
    try {
        app->exit();
    }
    catch (...)
    {
        cerr << "Failed to exit." << endl;
        return 1;
    }
    cout << "Bye." << endl;
    return -1;
}

int handleLoad(string filename, unordered_map<string, int> commands, MyApp* app)
{
    ifstream fin(filename.c_str());
    if (!fin)
    {
        cerr << "Error: cannot open file: " << filename << endl;
        return 1;
    }

    // read input file line by line
    string line;
    string action;
    while (getline(fin, line))
    {
        istringstream iss(line);
        // save the action command
        iss >> action;
        // split the inputs after the action command
        vector<string> inputs{istream_iterator<string>{iss}, istream_iterator<string>{}};

        if(handleInputs(action, inputs, commands, app) == -1)
            break;
    }

    fin.close();
    return 0;
}

int handleInputs(string action, vector<string>& inputs, unordered_map<string, int> commands, MyApp* app)
{
    if (commands.find(action) == commands.end())
    {
        cerr << "Input Error: command '" << action << "' not found" << endl;
        return 1;
    }

    int numArgs = commands[action];
    int inputSize = inputs.size();
    if (action == "gpa" && inputSize != 1 && inputSize != 2)
    {
        cerr << "Input Error: expect 1 or 2 arguments after 'gpa' command" << endl;
        return 1;
    }
    else if (action != "gpa" && inputSize != numArgs)
    {
        cerr << "Input Error: expect " << numArgs << " arguments after '" << action << "' command " << endl;
        return 1;
    }

    if (action == "ins")
    {
        return handleIns(inputs, app);
    }
    else if (action == "find" || action == "sfind")
    {
       return handleFind(action, inputs[0], app);
    }
    else if (action == "range")
    {
        return handleRange(inputs[0], inputs[1], app);
    }
    else if (action == "gpa")
    {
        if (inputSize == 2)
            return handleGpa(inputs[0], inputs[1], app);
        else
            return handleGpa(inputs[0], "", app);
    }
    else if (action == "del")
    {
        return handleDel(inputs[0], app);
    }
    else if (action == "print")
    {
        return handlePrint(app);
    }
    else if (action == "exit")
    {
        return handleExit(app);
    }
    else if (action == "load")
    {
        return handleLoad(inputs[0], commands, app);
    }

    // error if program reaches this point
    return 1;
}

