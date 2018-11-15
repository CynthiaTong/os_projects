#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <unordered_map>

#include "myapp.h"

#ifndef IO_H
#define IO_H

int handleInputs(std::string action, std::vector<std::string>& inputs, std::unordered_map<std::string, int> commands, MyApp* app);
int handleIns(std::vector<std::string>& inputs, MyApp* app);
int handleFind(std::string action, std::string id, MyApp* app);
int handleRange(std::string id1, std::string id2, MyApp* app);
int handleGpa(std::string id1, std::string id2, MyApp* app);
int handleDel(std::string id, MyApp* app);
int handlePrint(MyApp* app);
int handleExit(MyApp* app);
int handleLoad(std::string filename, std::unordered_map<std::string, int> commands, MyApp* app);

#endif
