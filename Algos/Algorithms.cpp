#include "fcfs.h"
#include "rr.h"
#include "sjf.h"
#include "LRTF.h"
#include "MLFQ.h"
#include <algorithm>
#include <queue>
#include <climits>

using namespace std;

void firstComeFirstServe(Process p[], int n, vector<tuple<int,int,int>>& gantt) {
    int time = 0;
    // Sort by arrival time
    vector<int> idx(n);
    for(int i=0; i<n; i++) idx[i] = i;
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return p[a].at < p[b].at;
    });

    for (int i = 0; i < n; i++) {
        int id = idx[i];
        if (time < p[id].at) time = p[id].at;
        gantt.push_back({time, p[id].pid, p[id].bt});
        time += p[id].bt;
        p[id].tat = time - p[id].at;
        p[id].wt = p[id].tat - p[id].bt;
    }
}

void roundRobin(Process p[], int n, int tq, vector<tuple<int,int,int>>& gantt) {
    for (int i = 0; i < n; i++) p[i].rt = p[i].bt;
    queue<int> q;
    int time = 0, completed = 0;
    int curProc = -1, curUsed = 0;
    vector<bool> inSys(n, false);

    while (completed < n) {
        // 1. Arrivals
        for (int i = 0; i < n; i++) {
            if (p[i].at == time && !inSys[i] && p[i].rt > 0) {
                q.push(i);
                inSys[i] = true;
            }
        }

        // 2. Quantum check
        if (curProc != -1 && curUsed >= tq) {
            q.push(curProc);
            curProc = -1;
        }

        // 3. Selection
        if (curProc == -1 && !q.empty()) {
            curProc = q.front();
            q.pop();
            curUsed = 0;
        }

        // 4. Execution
        if (curProc != -1) {
            if (!gantt.empty() && get<1>(gantt.back()) == p[curProc].pid && get<0>(gantt.back()) + get<2>(gantt.back()) == time)
                get<2>(gantt.back())++;
            else
                gantt.push_back({time, p[curProc].pid, 1});

            p[curProc].rt--;
            curUsed++;
            time++;

            if (p[curProc].rt == 0) {
                p[curProc].tat = time - p[curProc].at;
                p[curProc].wt = p[curProc].tat - p[curProc].bt;
                completed++;
                curProc = -1;
            }
        } else {
            int nextAt = INT_MAX;
            for (int i = 0; i < n; i++) if (p[i].rt > 0 && p[i].at > time) nextAt = min(nextAt, p[i].at);
            if (nextAt != INT_MAX) time = nextAt;
            else time++;
        }
    }
}

void shortestJobFirst(Process p[], int n, vector<tuple<int,int,int>>& gantt) {
    for (int i = 0; i < n; i++) p[i].rt = p[i].bt;
    int time = 0, completed = 0;
    vector<bool> done(n, false);

    while (completed < n) {
        int idx = -1, minBt = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!done[i] && p[i].at <= time && p[i].bt < minBt) {
                minBt = p[i].bt;
                idx = i;
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        gantt.push_back({time, p[idx].pid, p[idx].bt});
        time += p[idx].bt;
        p[idx].tat = time - p[idx].at;
        p[idx].wt = p[idx].tat - p[idx].bt;
        done[idx] = true;
        completed++;
    }
}

void shortestRemainingTime(Process p[], int n, vector<tuple<int,int,int>>& gantt) {
    for (int i = 0; i < n; i++) p[i].rt = p[i].bt;
    int time = 0, completed = 0;

    while (completed < n) {
        int idx = -1, minRt = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].at <= time && p[i].rt > 0 && p[i].rt < minRt) {
                minRt = p[i].rt;
                idx = i;
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        if (!gantt.empty() && get<1>(gantt.back()) == p[idx].pid) {
            get<2>(gantt.back())++;
        } else {
            gantt.push_back({time, p[idx].pid, 1});
        }

        p[idx].rt--;
        time++;

        if (p[idx].rt == 0) {
            p[idx].tat = time - p[idx].at;
            p[idx].wt = p[idx].tat - p[idx].bt;
            completed++;
        }
    }
}

void lrtfScheduling(Process p[], int n, vector<tuple<int,int,int>>& gantt) {
    for (int i = 0; i < n; i++) p[i].rt = p[i].bt;
    int time = 0, completed = 0;

    while (completed < n) {
        int idx = -1, maxRt = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].at <= time && p[i].rt > 0 && p[i].rt > maxRt) {
                maxRt = p[i].rt;
                idx = i;
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        if (!gantt.empty() && get<1>(gantt.back()) == p[idx].pid) {
            get<2>(gantt.back())++;
        } else {
            gantt.push_back({time, p[idx].pid, 1});
        }

        p[idx].rt--;
        time++;

        if (p[idx].rt == 0) {
            p[idx].tat = time - p[idx].at;
            p[idx].wt = p[idx].tat - p[idx].bt;
            completed++;
        }
    }
}

void mlfqScheduling(Process p[], int n, int quantum, vector<tuple<int,int,int>>& gantt) {
    for (int i = 0; i < n; i++) p[i].rt = p[i].bt;
    
    queue<int> q1, q2, q3;
    int q1_tq = quantum;
    int q2_tq = quantum * 2;
    
    int time = 0, completed = 0;
    int curProc = -1, curQ = 0, curUsed = 0;
    vector<bool> inSys(n, false);

    while (completed < n) {
        // 1. Arrivals
        for (int i = 0; i < n; i++) {
            if (p[i].at == time && !inSys[i] && p[i].rt > 0) {
                q1.push(i);
                inSys[i] = true;
            }
        }

        // 2. Quantum check (Feedback)
        if (curProc != -1) {
            bool expired = false;
            if (curQ == 1 && curUsed >= q1_tq) {
                q2.push(curProc);
                expired = true;
            } else if (curQ == 2 && curUsed >= q2_tq) {
                q3.push(curProc);
                expired = true;
            }
            if (expired) curProc = -1;
        }

        // 3. Priority Preemption
        if (curProc != -1) {
            bool pre = false;
            if (curQ == 2 && !q1.empty()) pre = true;
            if (curQ == 3 && (!q1.empty() || !q2.empty())) pre = true;
            
            if (pre) {
                if (curQ == 2) q2.push(curProc);
                else q3.push(curProc);
                curProc = -1;
            }
        }

        // 4. Selection
        if (curProc == -1) {
            if (!q1.empty())      { curProc = q1.front(); q1.pop(); curQ = 1; }
            else if (!q2.empty()) { curProc = q2.front(); q2.pop(); curQ = 2; }
            else if (!q3.empty()) { curProc = q3.front(); q3.pop(); curQ = 3; }
            curUsed = 0;
        }

        // 5. Execution
        if (curProc != -1) {
            if (!gantt.empty() && get<1>(gantt.back()) == p[curProc].pid && get<0>(gantt.back()) + get<2>(gantt.back()) == time)
                get<2>(gantt.back())++;
            else
                gantt.push_back({time, p[curProc].pid, 1});

            p[curProc].rt--;
            curUsed++;
            time++;

            if (p[curProc].rt == 0) {
                p[curProc].tat = time - p[curProc].at;
                p[curProc].wt = p[curProc].tat - p[curProc].bt;
                completed++;
                curProc = -1;
            }
        } else {
            int nextAt = INT_MAX;
            for (int i = 0; i < n; i++) if (p[i].rt > 0 && p[i].at > time) nextAt = min(nextAt, p[i].at);
            if (nextAt != INT_MAX) time = nextAt;
            else time++;
        }
    }
}
