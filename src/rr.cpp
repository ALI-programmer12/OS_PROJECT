#include "../include/rr.h"
#include <queue>
#include <iostream>
using namespace std;

void runRR(vector<Process>& list, int timeQuantum) {
    int n = static_cast<int>(list.size());

    for (auto &p : list) {
        p.remainingTime = p.burstTime;
        p.completed = false;
        p.inQueue = false;
    }

    queue<int> q;
    int time = 0;
    int completedCount = 0;

    while (completedCount < n) {
        for (int i = 0; i < n; i++) {
            if (!list[i].completed && !list[i].inQueue && list[i].arrivalTime <= time) {
                q.push(i);
                list[i].inQueue = true;
            }
        }

        if (q.empty()) {
            time++;
            continue;
        }

        int idx = q.front();
        q.pop();

        if (list[idx].remainingTime == list[idx].burstTime)
            list[idx].startTime = time;

        int exec = min(timeQuantum, list[idx].remainingTime);
        list[idx].remainingTime -= exec;
        time += exec;

        for (int i = 0; i < n; i++) {
            if (!list[i].completed && !list[i].inQueue && list[i].arrivalTime <= time) {
                q.push(i);
                list[i].inQueue = true;
            }
        }

        if (list[idx].remainingTime == 0) {
            list[idx].finishTime = time;
            list[idx].turnaroundTime = list[idx].finishTime - list[idx].arrivalTime;
            list[idx].waitingTime = list[idx].turnaroundTime - list[idx].burstTime;
            list[idx].completed = true;
            completedCount++;
        } else {
            q.push(idx);
        }
    }

    cout << "\nRound Robin Completed\n";
}
