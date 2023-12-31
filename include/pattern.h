#ifndef PATTERN_H
#define PATTERN_H

#include "smps.h"
#include <vector>

// Stochastic pattern for a single stage.
class StageStochasticPattern {
public:
    StageStochasticPattern() = default;
    StageStochasticPattern(int _stage, const std::vector<int> &_row_index,
                           const std::vector<int> &_col_index, const std::vector<double> &_reference_values,
                           const std::vector<size_t> &_indices_in_scenario);

    int stage;
    std::vector<int> row_index, col_index;
    std::vector<double> reference_values;
    std::vector<size_t> indices_in_scenario; // the indices in a full scenario vector in a multistage case.
    size_t rv_count;
};

// denotes where the randomness is in the current stage problem.
// currently it does not support cost row randomness
class StochasticPattern {
public:
    // constructor
    StochasticPattern(const std::vector<int> &_stage, const std::vector<int> &_row_index,
                      const std::vector<int> &_col_index, const std::vector<double> &_reference_values);

    // build the stochastic pattern from the smps data
    static StochasticPattern from_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim, const smps::SMPSStoch &sto);
    
    // keep only the random variables in the specified stage, discard the rest
    // returns the indices of the random elements belonging to that stage in the
    // original scenario vector
    StageStochasticPattern filter_by_stage(int stage);

    // stageness of each random variable
    std::vector<int> stage;

    // the indices of the random variables in the current stage problem
    // important: the variable indices are relative to the last stage,
    // but the row indices are relative to the current stage.
    // this can be used to reliably refer to elements in the transfer block
    // outside the transfer block, the following rule applies:
    // if the row_index is -1, it means the random element is in the cost coefficient
    // if the col_index is -1, it means the random element is in the rhs
    std::vector<int> row_index, col_index;

    // the default value of the random variable as in the deterministic template
    std::vector<double> reference_values;

    // the number of random variables in each stage
    std::vector<size_t> rv_count;

};

#endif // PATTERN_H