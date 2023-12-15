#ifndef PATTERN_H
#define PATTERN_H

#include "smps.h"
#include <vector>

// denotes where the randomness is in the current stage problem.
// currently it does not support cost row randomness
class StochasticPattern {
public:
    // constructor
    StochasticPattern(const std::vector<int> &_stage, const std::vector<int> &_row_index,
                      const std::vector<int> &_col_index, const std::vector<double> &_reference_values);

    // build the stochastic pattern from the smps data
    static StochasticPattern from_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim, const smps::SMPSStoch &sto);

    // stageness of each random variable
    std::vector<int> stage;

    // the indices of the random variables in the current stage problem
    // the indices are relative to the transfer block in the current stage problem
    // if the row_index is -1, it means the random element is in the cost coefficient
    // if the col_index is -1, it means the random element is in the rhs
    std::vector<int> row_index, col_index;

    // the default value of the random variable as in the deterministic template
    std::vector<double> reference_values;

    // the number of random variables in each stage
    std::vector<size_t> rv_count;

};

#endif // PATTERN_H