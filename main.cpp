#include <iostream>
#include <vector>
#include <limits>
#include "process.h"
#include "simulator.h"
#include "ranker.h"

using namespace std;

int main() {
    cout << "============================================================\n";
    cout << "         CPU SCHEDULING ALGORITHM SIMULATOR\n";
    cout << "============================================================\n";

    int n;
    cout << "\nEnter number of processes: ";
    cin >> n;

    Process p[n];

    // Check if priority needed
    int hasPriority;
    cout << "Do processes have priority values? (1=Yes, 0=No): ";
    cin >> hasPriority;

    cout << "\nEnter process details:\n";
    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;
        cout << "\nProcess P" << i+1 << ":\n";
        cout << "  Arrival Time : "; cin >> p[i].at;
        cout << "  Burst Time   : "; cin >> p[i].bt;
        if (hasPriority) {
            cout << "  Priority (lower = higher priority): "; cin >> p[i].pr;
        } else {
            p[i].pr = 0;
        }
        p[i].rt = p[i].bt;
        p[i].wt = 0;
        p[i].tat = 0;
    }

    int rqTimequantum = 4;
    cout << "\nEnter Round Robin time quantum (default=4): ";
    cin >> rqTimequantum;

    cout << "\n\nRunning all scheduling algorithms...\n";

    // Run all algorithms
    simulate(p, n, "1. FCFS (First Come First Serve)",           1);
    simulate(p, n, "2. Round Robin (RR)",                        2, rqTimequantum);
    simulate(p, n, "3. SJF Non-Preemptive",                      3);
    simulate(p, n, "4. SRTF (Shortest Remaining Time First)",    4);
    simulate(p, n, "5. LRTF (Longest Remaining Time First)",     5);
    simulate(p, n, "6. MLFQ (Multi-Level Feedback Queue)",       6);

    if (hasPriority) {
        simulate(p, n, "7. Preemptive Priority",                 7);
        simulate(p, n, "8. Non-Preemptive Priority",             8);
    }

    // Rank
    rankAlgorithms();

    return 0;
}
