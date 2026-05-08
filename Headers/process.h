#pragma once

struct Process {
    int pid = 0;   /* process id                  */
    int at  = 0;   /* arrival time                */
    int bt  = 0;   /* burst time (original)       */
    int rt  = 0;   /* remaining time (simulator)  */
    int pr  = 0;   /* priority (lower = higher)   */
    int wt  = 0;   /* waiting time  (output)      */
    int tat = 0;   /* turnaround    (output)       */
};
