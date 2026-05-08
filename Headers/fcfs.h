#ifndef FCFS_H
#define FCFS_H
#include "process.h"
#include <vector>
#include <tuple>

void firstComeFirstServe(Process p[], int n, std::vector<std::tuple<int,int,int>>& gantt);

#endif