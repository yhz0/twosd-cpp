// Single machine version of TwoSD using OpenMP

#ifndef TWO_STAGE_H
#define TWO_STAGE_H

#include <cstddef>
#include <omp.h>
#include <memory> // unique_ptr

#include "smps.h"
#include "prob.h"

class TwoStageSP
{
public:
    TwoStageSP(const std::string base_path, const std::string prob_name, int nworkers_);

private:
    // number of workers
    int nworkers;

    // Stage Problem
    // root stage problem
    std::unique_ptr<StageProblem> prob0;

    // second stage problem: one for each worker
    std::vector<std::unique_ptr<StageProblem>> prob1;

    // solve the problem
    virtual void solve() = 0;
};

class TwoStageSCS : public TwoStageSP
{
public:
    using TwoStageSP::TwoStageSP;

};

#endif // TWO_STAGE_H