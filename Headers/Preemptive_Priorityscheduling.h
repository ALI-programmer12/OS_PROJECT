#ifndef PREEMPTIVE_PRIORITY_H
#define PREEMPTIVE_PRIORITY_H
#include "process.h"
#include <vector>
#include <tuple>
void preemptivePriorityScheduling(Process p[], int n, std::vector<std::tuple<int,int,int>>& gantt);
#endif
