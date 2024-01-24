// Single machine version of TwoSD using OpenMP

#ifndef TWO_STAGE_H
#define TWO_STAGE_H

#include <cstddef>
#include <omp.h>
#include <memory> // unique_ptr

#include "smps.h"
#include "prob.h"
#include "cut_helper.h"
#include "projection.h"

class TwoStageSP
{
public:
    TwoStageSP(const std::string base_path, const std::string prob_name, int nworkers_);

protected:
    // number of workers
    int nworkers;

    // Stage Problem
    // root stage problem
    std::unique_ptr<StageProblem> prob0;

    // second stage problem: one for each worker
    std::vector<std::unique_ptr<StageProblem>> prob1;

    // solve the problem
    virtual void solve() = 0;

    // smps data
    smps::SMPSCore cor;
    smps::SMPSImplicitTime tim;
    smps::SMPSStoch sto;

};

class TwoStageSCS : public TwoStageSP
{
public:
    TwoStageSCS(const std::string base_path, const std::string prob_name, int nworkers_);
    void solve() override;

private:
    // problem used for projection
    std::unique_ptr<StageProjectionProblem> proj_prob0;

    // solve all subproblems, returning a list of solutions
    std::pair <
        std::vector<StageProblem::Solution>,
        std::vector<Cut> > solve_subproblems(const std::vector<double> &x, const std::vector<std::vector<double>> &samples);
};

#endif // TWO_STAGE_H