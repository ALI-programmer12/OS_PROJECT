#include "simulator.h"
#include "fcfs.h"
#include "rr.h"
#include "sjf.h"
#include "LRTF.h"
#include "MLFQ.h"
#include "Non-Preemptive_Priorityscheduling.h"
#include "Preemptive_Priorityscheduling.h"
#include <algorithm>
#include <vector>

using namespace std;

vector<Result> results;

// Helper to merge consecutive Gantt slices for the same process
static vector<tuple<int,int,int>> mergeGantt(const vector<tuple<int,int,int>>& raw) {
    if (raw.empty()) return raw;
    vector<tuple<int,int,int>> out;
    for (auto &[s, pid, d] : raw) {
        if (!out.empty() && get<1>(out.back()) == pid && get<0>(out.back()) + get<2>(out.back()) == s) {
            get<2>(out.back()) += d;
        } else {
            out.push_back({s, pid, d});
        }
    }
    return out;
}

static void calcAvg(Process* p, int n, double& awt, double& atat) {
    double tw = 0, tt = 0;
    for (int i = 0; i < n; i++) { tw += p[i].wt; tt += p[i].tat; }
    awt = (n > 0) ? tw / n : 0;
    atat = (n > 0) ? tt / n : 0;
}

void simulate(Process* orig, int n, const std::string &name, int algoId, int quantum) {
    if (n <= 0 || !orig) return;
    
    
    int totalBurst = 0;
    for (int i = 0; i < n; i++) totalBurst += orig[i].bt;
    if (totalBurst > 10000) return;  
    
    Process* p = new Process[n];
    for (int i = 0; i < n; i++) p[i] = orig[i];

    Result res;
    res.name = name;
    res.gantt.clear();

    // Using the external functions consistently
    if (algoId == 1) firstComeFirstServe(p, n, res.gantt);
    else if (algoId == 2) roundRobin(p, n, quantum, res.gantt);
    else if (algoId == 3) shortestJobFirst(p, n, res.gantt);
    else if (algoId == 4) shortestRemainingTime(p, n, res.gantt);
    else if (algoId == 5) lrtfScheduling(p, n, res.gantt);
    else if (algoId == 6) mlfqScheduling(p, n, quantum, res.gantt);
    else if (algoId == 7) preemptivePriorityScheduling(p, n, res.gantt);
    else if (algoId == 8) nonPreemptivePriorityScheduling(p, n, res.gantt);

    res.gantt = mergeGantt(res.gantt);
    calcAvg(p, n, res.avgWT, res.avgTAT);
    res.procs.assign(p, p + n);
    results.push_back(res);
    delete[] p;
}