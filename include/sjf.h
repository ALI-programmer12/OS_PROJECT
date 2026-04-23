#ifndef SJF_H
#define SJF_H
#include "process.h"
void shortestJobFirst(Process p[], int n);       // Non-preemptive SJF
void shortestRemainingTime(Process p[], int n);  // Preemptive SJF (SRTF)
#endif
