#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <vector>
#include <utility>
#include <tuple>

struct Result {
    std::string name;
    float avgWT;
    float avgTAT;
    std::vector<std::tuple<int, int, int>> gantt;
    Result(std::string n, float wt, float tat, std::vector<std::tuple<int, int, int>> g) : name(n), avgWT(wt), avgTAT(tat), gantt(g) {}
};

#endif
