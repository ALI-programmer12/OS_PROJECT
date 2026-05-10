#include "ranker.h"
#include "simulator.h"
#include <algorithm>

// Sorts results in-place by ascending avgWT (best first)
void rankAlgorithms() {
    std::sort(results.begin(), results.end(),
              [](const Result &a, const Result &b) {
                  return a.avgWT < b.avgWT;
              });
}
