#ifndef NON_PREEMPTIVE_PRIORITY_H
#define NON_PREEMPTIVE_PRIORITY_H
#include "process.h"
#include <vector>
#include <tuple>
void nonPreemptivePriorityScheduling(Process p[], int n, std::vector<std::tuple<int,int,int>>& gantt);
#endif
