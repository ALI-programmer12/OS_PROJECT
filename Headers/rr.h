#ifndef RR_H
#define RR_H
#include "process.h"
#include <vector>
#include <tuple>
void roundRobin(Process p[], int n, int tq, std::vector<std::tuple<int,int,int>>& gantt);
#endif