#include "Non-Preemptive_Priorityscheduling.h"
#include <algorithm>
#include <climits>
using namespace std;

void nonPreemptivePriorityScheduling(Process p[], int n, vector<tuple<int,int,int>>& gantt) {
    vector<bool> done(n, false);
    int time = 0, completed = 0;
    while (completed < n) {
        int sel = -1, best = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!done[i] && p[i].at <= time && p[i].pr < best) {
                best = p[i].pr; sel = i;
            }
        }
        if (sel == -1) { time++; continue; }
        gantt.push_back({time, p[sel].pid, p[sel].bt});
        p[sel].wt  = time - p[sel].at;
        time      += p[sel].bt;
        p[sel].tat = time - p[sel].at;
        done[sel]  = true;
        completed++;
    }
}
