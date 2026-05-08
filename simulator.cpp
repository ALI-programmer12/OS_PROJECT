#include "simulator.h"
#include "LRTF.h"
#include "Non-Preemptive_Priorityscheduling.h"
#include "Preemptive_Priorityscheduling.h"
#include <algorithm>
#include <climits>
#include <queue>
#include <cstring>
using namespace std;

// Global results list (declared extern in simulator.h)
vector<Result> results;

// ---------------------------------------------------------------------------
// Helper: merge consecutive single-unit Gantt slices of same pid
// ---------------------------------------------------------------------------
static vector<tuple<int,int,int>> mergeGantt(const vector<tuple<int,int,int>>& raw) {
    vector<tuple<int,int,int>> out;
    for (auto& [s, pid, d] : raw) {
        if (!out.empty() && get<1>(out.back()) == pid &&
            get<0>(out.back()) + get<2>(out.back()) == s) {
            get<2>(out.back()) += d;
        } else {
            out.push_back({s, pid, d});
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// Helper: compute avg WT / TAT from process array after scheduling
// ---------------------------------------------------------------------------
static void calcAvg(Process* p, int n, double& awt, double& atat) {
    double tw = 0, tt = 0;
    for (int i = 0; i < n; i++) { tw += p[i].wt; tt += p[i].tat; }
    awt  = tw / n;
    atat = tt / n;
}

// ---------------------------------------------------------------------------
// FCFS  (algoId = 1)
// ---------------------------------------------------------------------------
static void runFCFS(Process* p, int n, vector<tuple<int,int,int>>& gantt) {
    // Sort by arrival time
    vector<int> order(n);
    for (int i = 0; i < n; i++) order[i] = i;
    sort(order.begin(), order.end(), [&](int a, int b){ return p[a].at < p[b].at; });

    int time = 0;
    for (int idx : order) {
        if (time < p[idx].at) {
            gantt.push_back({time, 0, p[idx].at - time}); // idle
            time = p[idx].at;
        }
        gantt.push_back({time, p[idx].pid, p[idx].bt});
        time += p[idx].bt;
        p[idx].tat = time - p[idx].at;
        p[idx].wt  = p[idx].tat - p[idx].bt;
    }
}

// ---------------------------------------------------------------------------
// Round Robin  (algoId = 2)
// ---------------------------------------------------------------------------
static void runRR(Process* p, int n, int quantum, vector<tuple<int,int,int>>& gantt) {
    vector<int> remBT(n);
    for (int i = 0; i < n; i++) remBT[i] = p[i].bt;

    queue<int> rq;
    vector<bool> inQueue(n, false);
    int time = 0, done = 0;

    // Enqueue processes that arrive at time 0
    for (int i = 0; i < n; i++)
        if (p[i].at == 0) { rq.push(i); inQueue[i] = true; }

    while (done < n) {
        if (rq.empty()) {
            // Find next arrival
            int nextAT = INT_MAX;
            for (int i = 0; i < n; i++)
                if (!inQueue[i] && remBT[i] > 0) nextAT = min(nextAT, p[i].at);
            gantt.push_back({time, 0, nextAT - time});
            time = nextAT;
            for (int i = 0; i < n; i++)
                if (!inQueue[i] && p[i].at <= time && remBT[i] > 0) { rq.push(i); inQueue[i] = true; }
            continue;
        }

        int idx = rq.front(); rq.pop();
        int slice = min(quantum, remBT[idx]);
        gantt.push_back({time, p[idx].pid, slice});
        time += slice;
        remBT[idx] -= slice;

        // Enqueue newly arrived processes
        for (int i = 0; i < n; i++)
            if (!inQueue[i] && p[i].at <= time && remBT[i] > 0) { rq.push(i); inQueue[i] = true; }

        if (remBT[idx] > 0) {
            rq.push(idx);
        } else {
            p[idx].tat = time - p[idx].at;
            p[idx].wt  = p[idx].tat - p[idx].bt;
            done++;
        }
    }
}

// ---------------------------------------------------------------------------
// SJF Non-Preemptive  (algoId = 3)
// ---------------------------------------------------------------------------
static void runSJF(Process* p, int n, vector<tuple<int,int,int>>& gantt) {
    vector<bool> done(n, false);
    int time = 0, completed = 0;
    while (completed < n) {
        int idx = -1;
        for (int i = 0; i < n; i++)
            if (!done[i] && p[i].at <= time)
                if (idx == -1 || p[i].bt < p[idx].bt) idx = i;

        if (idx == -1) {
            int nextAT = INT_MAX;
            for (int i = 0; i < n; i++)
                if (!done[i]) nextAT = min(nextAT, p[i].at);
            gantt.push_back({time, 0, nextAT - time});
            time = nextAT;
            continue;
        }
        gantt.push_back({time, p[idx].pid, p[idx].bt});
        time += p[idx].bt;
        p[idx].tat = time - p[idx].at;
        p[idx].wt  = p[idx].tat - p[idx].bt;
        done[idx]  = true;
        completed++;
    }
}

// ---------------------------------------------------------------------------
// SRTF — Shortest Remaining Time First  (algoId = 4)
// ---------------------------------------------------------------------------
static void runSRTF(Process* p, int n, vector<tuple<int,int,int>>& gantt) {
    // Initialize remaining time and metrics
    for (int i = 0; i < n; i++) { p[i].rt = p[i].bt; p[i].wt = 0; p[i].tat = 0; }

    int time = 0, done = 0;
    int lastPid = -1, segStart = 0;

    auto flush = [&](int end) {
        if (lastPid != -1 && end > segStart)
            gantt.push_back({segStart, lastPid, end - segStart});
    };

    while (done < n) {
        int idx = -1, best = INT_MAX;
        for (int i = 0; i < n; i++)
            if (p[i].at <= time && p[i].rt > 0 && p[i].rt < best) { best = p[i].rt; idx = i; }

        if (idx == -1) {
            flush(time);
            int nextAT = INT_MAX;
            for (int i = 0; i < n; i++)
                if (p[i].rt > 0) nextAT = min(nextAT, p[i].at);
            gantt.push_back({time, 0, nextAT - time});
            segStart = nextAT; lastPid = -1;
            time = nextAT;
            continue;
        }

        if (p[idx].pid != lastPid) { flush(time); segStart = time; lastPid = p[idx].pid; }
        p[idx].rt--;
        time++;
        if (p[idx].rt == 0) {
            flush(time); segStart = time; lastPid = -1;
            p[idx].tat = time - p[idx].at;
            p[idx].wt  = p[idx].tat - p[idx].bt;
            done++;
        }
    }
}

// ---------------------------------------------------------------------------
// MLFQ — Multi-Level Feedback Queue  (algoId = 6)
// Levels: Q0 (quantum), Q1 (quantum*2), Q2 (FCFS)
// ---------------------------------------------------------------------------
static void runMLFQ(Process* p, int n, int quantum, vector<tuple<int,int,int>>& gantt) {
    struct MLProc { int idx; int level; int remBT; int usedInSlice; };
    vector<queue<MLProc>> qs(3);
    vector<bool> admitted(n, false);
    int time = 0, done = 0;
    int quanta[3] = { quantum, quantum * 2, INT_MAX };

    auto admitArrivals = [&]() {
        for (int i = 0; i < n; i++)
            if (!admitted[i] && p[i].at <= time) {
                qs[0].push({i, 0, p[i].bt, 0});
                admitted[i] = true;
            }
    };
    admitArrivals();

    while (done < n) {
        int level = -1;
        for (int l = 0; l < 3; l++) if (!qs[l].empty()) { level = l; break; }

        if (level == -1) {
            // Idle gap
            int nextAT = INT_MAX;
            for (int i = 0; i < n; i++)
                if (!admitted[i]) nextAT = min(nextAT, p[i].at);
            gantt.push_back({time, 0, nextAT - time});
            time = nextAT;
            admitArrivals();
            continue;
        }

        MLProc cur = qs[level].front(); qs[level].pop();
        int slice = min(quanta[level], cur.remBT);
        gantt.push_back({time, p[cur.idx].pid, slice});
        time += slice;
        cur.remBT -= slice;
        admitArrivals();

        if (cur.remBT == 0) {
            p[cur.idx].tat = time - p[cur.idx].at;
            p[cur.idx].wt  = p[cur.idx].tat - p[cur.idx].bt;
            done++;
        } else {
            int nextLevel = min(level + 1, 2);
            cur.level = nextLevel;
            qs[nextLevel].push(cur);
        }
    }
}

// ---------------------------------------------------------------------------
// simulate() — entry point called from main.cpp
// algoId: 1=FCFS, 2=RR, 3=SJF, 4=SRTF, 5=NonPre-Priority, 6=MLFQ, 7=Pre-Priority
// ---------------------------------------------------------------------------
void simulate(Process* orig, int n, const std::string &name, int algoId, int quantum) {
    // Ensure quantum is positive for RR and MLFQ
    if (quantum <= 0) quantum = 1;

    // Work on a copy so original data is not mutated
    Process* p = new Process[n];
    for (int i = 0; i < n; i++) p[i] = orig[i];

    Result res;
    res.name = name;

    switch (algoId) {
        case 1: runFCFS(p, n, res.gantt); break;
        case 2: runRR  (p, n, quantum, res.gantt); break;
        case 3: runSJF (p, n, res.gantt); break;
        case 4: runSRTF(p, n, res.gantt); break;
        case 5: lrtfScheduling(p, n, res.gantt); break;
        case 6: runMLFQ(p, n, quantum, res.gantt); break;
        case 7: preemptivePriorityScheduling(p, n, res.gantt); break;
        case 8: nonPreemptivePriorityScheduling(p, n, res.gantt); break;
    }

    res.gantt = mergeGantt(res.gantt);
    calcAvg(p, n, res.avgWT, res.avgTAT);
    res.procs.assign(p, p + n);
    results.push_back(res);
    delete[] p;
}
