#ifndef MLFQ_H
#define MLFQ_H
#include "process.h"
#include <vector>
#include <tuple>
void mlfqScheduling(Process p[], int n, int quantum, std::vector<std::tuple<int,int,int>>& gantt);
#endif
