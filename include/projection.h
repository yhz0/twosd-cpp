#ifndef PROJECTION_H
#define PROJECTION_H

#include "prob.h"
#include <optional>

class StageProjectionProblem: public StageProblem {
    public:

    using StageProblem::StageProblem;

    void attach_solver() override;

    // project the given vector to the feasible set
    // if the vector is already feasible, return nullopt
    // otherwise, return the difference between the projected vector and the given vector d
    // such that the projected vector is x0 + d
    std::optional<std::vector<double>> solve(const std::vector<double> &x0);

    private:
    // remove linear terms from the objective
    void remove_linear_terms();

    // check if the given vector is feasible
    bool is_feasible(const std::vector<double> &x0);
};

#endif