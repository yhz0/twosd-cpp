#ifndef PROB_H
#define PROB_H

#include "smps.h"
#include "pattern.h"    // for StageStochasticPattern
#include <memory>   // for std::unique_ptr

// TODO: implement gurobi solver
class Solver {};

class StageProblem
{
public:
   StageProblem(
        size_t nvars_last_,
        size_t nvars_current_,
        size_t nrows_,
        const std::vector<std::string>& last_stage_var_names_,
        const std::vector<std::string>& current_stage_var_names_,
        const std::vector<std::string>& current_stage_row_names_,
        const SparseMatrix<double>& transfer_block_,
        const SparseMatrix<double>& current_block_,
        const std::vector<double>& lb_,
        const std::vector<double>& ub_,
        const std::vector<double>& x_base_,
        const std::vector<double>& rhs_bar_,
        const std::vector<double>& rhs_shift_,
        const std::vector<char>& inequality_directions_,
        const std::vector<double>& cost_,
        double cost_shift_,
        const StageStochasticPattern& stage_stoc_pattern_
    ) : nvars_last(nvars_last_),
        nvars_current(nvars_current_),
        nrows(nrows_),
        last_stage_var_names(last_stage_var_names_),
        current_stage_var_names(current_stage_var_names_),
        current_stage_row_names(current_stage_row_names_),
        transfer_block(transfer_block_),
        current_block(current_block_),
        lb(lb_),
        ub(ub_),
        x_base(x_base_),
        rhs_bar(rhs_bar_),
        rhs_shift(rhs_shift_),
        inequality_directions(inequality_directions_),
        cost(cost_),
        cost_shift(cost_shift_),
        stage_stoc_pattern(stage_stoc_pattern_),
        solver(nullptr)  // Initialize solver to nullptr
    {}

    // copy the lp coefficients from the core file and time file
    // and store the sparsity pattern in the sto file.
    // solver is not initialized
    static StageProblem from_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim,
                                  const smps::SMPSStoch &sto, int stage);

private:
    // length of last stage variables (columns)
    // in this representation, we assume that the column numbers are integers and consecutive
    // the last stage variables comes first, then the current stage variables
    size_t nvars_last;

    // length of current stage variables
    size_t nvars_current;

    // length of current stage constraints, excluding the number of bounds
    size_t nrows;

    // names of last stage variables
    std::vector<std::string> last_stage_var_names;

    // names of current stage variables
    std::vector<std::string> current_stage_var_names;

    // names of current stage constraints
    std::vector<std::string> current_stage_row_names;

    // LP coefficients
    // transfer_block: (nrows, nvars_last) matrix
    // current_block: (nrows, nvars_current) matrix
    SparseMatrix<double> transfer_block, current_block;

    // lb, ub: lower/upper bound of the current stage variables
    // (nvars_current, )
    std::vector<double> lb, ub;

    // everything is denoted in d-space with a change of variables d = x - x_base
    // this denotes the current x_base (nvars_current, )
    // this term is zero for terminal stage
    std::vector<double> x_base;

    // fixed part of current rhs, shift part of current rhs
    // Ax = A (d+x_base) <= b  === Ad <= b-A*x_base
    // rhs_bar = b (n_rows, )
    // rhs_shift = A*x_base (n_rows, )
    // so the true rhs should be rhs_bar - A*x_base
    std::vector<double> rhs_bar, rhs_shift;

    // directions of inequalities in current stage
    // should be one of G, L, E
    // (nrows, )
    std::vector<char> inequality_directions;

    // cost coefficients of current stage
    // (nvars_current, )
    std::vector<double> cost;

    // shift of current coefficient.
    // c*x = c*(d+x_base) = c*d + c*x_base
    // This term denotes c*x_base part.
    double cost_shift;

    // position of random elements in the transfer block or RHS
    StageStochasticPattern stage_stoc_pattern;

    // the pointer to the solver environment
    std::unique_ptr<Solver> solver;
};

#endif // PROB_H
