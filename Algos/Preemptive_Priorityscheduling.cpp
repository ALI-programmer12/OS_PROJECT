#include "Preemptive_Priorityscheduling.h"
#include <algorithm>
#include <climits>
using namespace std;

void preemptivePriorityScheduling(Process p[], int n, vector<tuple<int,int,int>>& gantt) {
    for (int i = 0; i < n; i++) p[i].rt = p[i].bt;
    int time = 0, completed = 0;
    while (completed < n) {
        int sel = -1, best = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].at <= time && p[i].rt > 0 && p[i].pr < best) {
                best = p[i].pr; sel = i;
            }
        }
        if (sel == -1) { time++; continue; }
        if (!gantt.empty() && get<1>(gantt.back()) == p[sel].pid)
            get<2>(gantt.back())++;
        else
            gantt.push_back({time, p[sel].pid, 1});
        p[sel].rt--;
        time++;
        if (p[sel].rt == 0) {
            p[sel].tat = time - p[sel].at;
            p[sel].wt  = p[sel].tat - p[sel].bt;
            completed++;
        }
    }
}
