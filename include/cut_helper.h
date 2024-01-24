#ifndef CUT_HELPER_H
#define CUT_HELPER_H

#include "prob.h"
#include <vector>

struct Cut {
    double alpha;
    std::vector<double> beta;
};

// helper functions for cutting plane algorithms
class CutHelper
{
    public:
    // get the static part of the cut
    // prob: second (or next) stage problem
    // alpha = rhs_bar * pi (bounds included)
    // beta = transpose(transfer_block) * pi
    static Cut get_static_part(const StageProblem& prob, const std::vector<double>& pi);

    // add the dynamic part of the cut
    // prob: second (or next) stage problem
    // alpha += rhs_delta * pi
    // beta += transpose(current_block) * pi
    static void add_dynamic_part(const StageProblem& prob, const std::vector<double>& pi, const std::vector<double>& scenario, Cut& cut);

};
#endif // CUT_HELPER_H