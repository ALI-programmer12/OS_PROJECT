#ifndef LRTF_H
#define LRTF_H
#include "process.h"
#include <vector>
#include <tuple>
void lrtfScheduling(Process p[], int n, std::vector<std::tuple<int,int,int>>& gantt);
#endif
