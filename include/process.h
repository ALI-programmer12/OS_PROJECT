#ifndef PROCESS_H
#define PROCESS_H

struct Process {
    int pid;
    union {
        int arrivalTime;
        int at;
    };
    union {
        int burstTime;
        int bt;
    };

    int startTime;
    int finishTime;
    union {
        int waitingTime;
        int wt;
    };
    union {
        int turnaroundTime;
        int tat;
    };

    int priority;
    union {
        int remainingTime;
        int rt;
    };

    bool completed;
    bool inQueue;
};

struct GanttBlock {
    int pid;
    int start;
    int end;
};

#endif
