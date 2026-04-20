#include "../include/sjf.h"
#include <iostream>
#include <climits>
using namespace std;

void runSJF(vector<Process>& list) {
    int n = list.size(); // number of processes

    for (auto &p : list)
        p.completed = false; // Marks all processes as not completed

    int completedCount = 0; // Tracks how many processes are done
    int time = 0; // simulation clock (CPU time starts at 0 which is in Ghant chart)

    while (completedCount < n) {
        int idx = -1;
        int minBurst = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (!list[i].completed && list[i].arrivalTime <= time) {
                if (list[i].burstTime < minBurst) {
                    minBurst = list[i].burstTime; // store the smallest burst time process
                    idx = i;
                } else if (list[i].burstTime == minBurst) // Tie b/w 2 processes then: 
                { 
                    if (idx == -1 || list[i].arrivalTime < list[idx].arrivalTime) //earlier arrival time wins
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
