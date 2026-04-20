#include "../include/sjf.h"
#include <iostream>
#include <limits>
using namespace std;

void runSJF(vector<Process>& list) {
    int n = static_cast<int>(list.size());

    for (auto &p : list)
        p.completed = false;

    int completedCount = 0;
    int time = 0;

    while (completedCount < n) {
        int idx = -1;
        int minBurst = numeric_limits<int>::max();

        for (int i = 0; i < n; i++) {
            if (!list[i].completed && list[i].arrivalTime <= time) {
                if (list[i].burstTime < minBurst) {
                    minBurst = list[i].burstTime;
                    idx = i;
                } else if (list[i].burstTime == minBurst) {
                    if (idx == -1 || list[i].arrivalTime < list[idx].arrivalTime)
                        idx = i;
                }
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        list[idx].startTime = time;
        list[idx].finishTime = time + list[idx].burstTime;
        list[idx].turnaroundTime = list[idx].finishTime - list[idx].arrivalTime;
        list[idx].waitingTime = list[idx].turnaroundTime - list[idx].burstTime;
        list[idx].completed = true;

        time = list[idx].finishTime;
        completedCount++;
    }

    cout << "\nSJF Completed\n";
}
