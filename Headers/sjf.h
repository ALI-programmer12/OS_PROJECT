#ifndef SJF_H
#define SJF_H
#include "process.h"
#include <vector>
#include <tuple>
void shortestJobFirst(Process p[], int n, std::vector<std::tuple<int,int,int>>& gantt);       // Non-preemptive SJF
void shortestRemainingTime(Process p[], int n, std::vector<std::tuple<int,int,int>>& gantt);  // Preemptive SJF (SRTF)
#endif
