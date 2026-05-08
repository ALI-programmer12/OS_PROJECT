#pragma once
#include <string>
#include <vector>
#include <tuple>
#include "process.h"

struct Result {
    std::string name;
    double avgWT  = 0;
    double avgTAT = 0;
    std::vector<std::tuple<int,int,int>> gantt;  /* (start, pid, duration) */
    std::vector<Process> procs;                  /* per-process results */
};

extern std::vector<Result> results;

/* algoId: 1=FCFS 2=RR 3=SJF 4=SRTF 5=LRTF 6=MLFQ 7=PrePriority 8=NPPriority */
void simulate(Process *p, int n, const std::string &name,
              int algoId, int quantum = 4);
