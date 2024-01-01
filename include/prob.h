#ifndef PROB_H
#define PROB_H

#include "smps.h"
#include "pattern.h"    // for StageStochasticPattern
#include <memory>   // for std::unique_ptr

class Solver;

class StageProblem
{
public:
    StageProblem(
        size_t nvars_last_,
        size_t nvars_current_,
        size_t nrows_,
        const std::vector<std::string> &last_stage_var_names_,
        const std::vector<std::string> &current_stage_var_names_,
        const std::vector<std::string> &current_stage_row_names_,
        const SparseMatrix<double> &transfer_block_,
        const SparseMatrix<double> &current_block_,
        const std::vector<double> &lb_,
        const std::vector<double> &ub_,
        const std::vector<double> &rhs_bar_,
        const std::vector<char> &inequality_directions_,
        const std::vector<double> &cost_coefficients_,
        const StageStochasticPattern &stage_stoc_pattern_) : nvars_last(nvars_last_),
                                                             nvars_current(nvars_current_),
                                                             nrows(nrows_),
                                                             last_stage_var_names(last_stage_var_names_),
                                                             current_stage_var_names(current_stage_var_names_),
                                                             current_stage_row_names(current_stage_row_names_),
                                                             transfer_block(transfer_block_),
                                                             current_block(current_block_),
                                                             lb(lb_),
                                                             ub(ub_),
                                                             rhs_bar(rhs_bar_),
                                                             inequality_directions(inequality_directions_),
                                                             cost_coefficients(cost_coefficients_),
                                                             stage_stoc_pattern(stage_stoc_pattern_),
                                                             // private member initializers
                                                             shift_x_base(false),
                                                             x_base(nvars_current_, 0.0),  // set zero for shifts
                                                             rhs_shift(nrows_, 0.0),
                                                             cost_shift(0.0),
                                                             solver(nullptr)
    {
    }

    // copy the lp coefficients from the core file and time file
    // and store the sparsity pattern in the sto file.
    // solver is not initialized
    static StageProblem from_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim,
                                  const smps::SMPSStoch &sto, int stage);

    // length of last stage variables (columns)
    // in this representation, we assume that the column numbers are integers and consecutive
    // the last stage variables comes first, then the current stage variables
    const size_t nvars_last;

    // length of current stage variables
    const size_t nvars_current;

    // length of current stage constraints, excluding the number of bounds
    const size_t nrows;

    // names of last stage variables
    const std::vector<std::string> last_stage_var_names;

    // names of current stage variables
    const std::vector<std::string> current_stage_var_names;

    // names of current stage constraints
    const std::vector<std::string> current_stage_row_names;

    // LP coefficients
    // transfer_block: (nrows, nvars_last) matrix
    // current_block: (nrows, nvars_current) matrix
    const SparseMatrix<double> transfer_block, current_block;

    // lb, ub: lower/upper bound of the current stage variables
    // (nvars_current, )
    const std::vector<double> lb, ub;

    // rhs as in the template
    // fixed part of the rhs
    const std::vector<double> rhs_bar;

    // directions of inequalities in current stage
    // should be one of G, L, E
    // (nrows, )
    const std::vector<char> inequality_directions;

    // cost coefficients of current stage
    // (nvars_current, )
    const std::vector<double> cost_coefficients;

    // position of random elements in the transfer block or RHS
    const StageStochasticPattern stage_stoc_pattern;

    // destructor
    ~StageProblem();

    private:
    // if shift_x_base is true,
    // everything is denoted in d-space with a change of variables d = x - x_base
    // this denotes the current x_base (nvars_current, )
    // rhs needs to be shifted too.
    // If we denote current stage variable as x,
    // and last stage variable as z (usually given in parameters)
    // Tz + Ax <= r =>  A (d+x_base) <= r  === Ad <= r -A*x_base - Tz
    // rhs_bar = r (n_rows, )
    // rhs_shift = A*x_base (n_rows, )
    // so the true rhs should be rhs_bar - A*x_base - Tz
    bool shift_x_base;
    std::vector<double> x_base, rhs_shift;

    // shift of current coefficient.
    // c*x = c*(d+x_base) = c*d + c*x_base
    // This term denotes c*x_base part.
    double cost_shift;

    // the pointer to the solver environment
    std::unique_ptr<Solver> solver;
};

#endif // PROB_H
