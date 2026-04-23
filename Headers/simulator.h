#ifndef SIMULATOR_H
#define SIMULATOR_H
#include "process.h"
#include "result.h"
#include <vector>
#include <string>
#include <tuple>
extern std::vector<Result> results;
extern std::vector<std::tuple<int, int, int>> currentGantt;
void simulate(Process p[], int n, std::string name, int algo, int quantum = 4);
#endif
