#include "io.h"
using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        cerr << "Usage: " << argv[0] << " -m MaxNumberOfPointers -v MaxValue" << endl;
        return 1;
    }

    // check for -m, -v flags, and assign given values to maxLevel and maxValue
    int maxLevel, maxValue;
    try
    {
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                switch(argv[i][1])
                {
                    case 'm':
                        maxLevel = stod(argv[i+1]);
                        break;
                    case 'v':
                        maxValue = stod(argv[i+1]);
                        break;
                }
            }
        }
    }
    catch (...)
    {
        cerr << "Invalid arguments: MaxNumberOfPointers and MaxValue must be integers" << endl;
        return 1;
    }

    MyApp* app = new MyApp(maxLevel, maxValue);
    // use an unordered map to keep track of the expected number of arguments for each valid command
    unordered_map<string, int> commands;
    commands["ins"] = 7;
    commands["find"] = 1;
    commands["sfind"] = 1;
    commands["range"] = 2;
    commands["gpa"] = 2;
    commands["del"] = 1;
    commands["print"] = 0;
    commands["load"] = 1;
    commands["exit"] = 0;

    // "infinite" user prompt
    while (1)
    {
        cout << "> ";

        string line;
        string action;

        getline(cin, line);
        istringstream iss(line);
        // save the action command
        iss >> action;
        // split the input arguments after the action command
        vector<string> inputs{istream_iterator<string>{iss}, istream_iterator<string>{}};
        // use the handleInputs function in io.cc
        // if return value is -1, the user has excecuted "exit"
        if(handleInputs(action, inputs, commands, app) == -1)
            break;
    }

    delete app;
    return 0;
}

