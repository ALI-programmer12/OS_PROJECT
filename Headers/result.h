#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <vector>
#include <utility>
#include <tuple>

struct Result {
    std::string name;
    double avgWT;
    double avgTAT;
    std::vector<std::tuple<int, int, int>> gantt;
    Result(std::string n, double wt, double tat, std::vector<std::tuple<int, int, int>> g) : name(n), avgWT(wt), avgTAT(tat), gantt(g) {}
};

#endif
