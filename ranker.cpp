#include "ranker.h"
#include "simulator.h"
#include <algorithm>
using namespace std;

/* Sort results by avgWT ascending (primary), avgTAT ascending (secondary). */
void rankAlgorithms() {
    sort(results.begin(), results.end(), [](const Result &a, const Result &b) {
        if (a.avgWT != b.avgWT) return a.avgWT < b.avgWT;
        return a.avgTAT < b.avgTAT;
    });
}
