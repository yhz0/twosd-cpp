// Single machine version of TwoSD using OpenMP

#ifndef TWOSD_SINGLE_H
#define TWOSD_SINGLE_H

#include <cstddef>
#include <omp.h>
#include <memory> // unique_ptr

#include "smps.h"
#include "prob.h"

class TwoSDSingle {
public:
    TwoSDSingle(const std::string base_path, const std::string prob_name, int nworkers_);

private:
    // number of workers
    int nworkers;

    // Stage Problem
    // root stage problem
    std::unique_ptr<StageProblem> prob0;

    // second stage problem: one for each worker
    std::vector<std::unique_ptr<StageProblem>> prob1;
};

#endif // TWOSD_SINGLE_H