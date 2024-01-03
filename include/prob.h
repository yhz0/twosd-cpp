#ifndef PROB_H
#define PROB_H

#include "smps.h"
#include "pattern.h"    // for StageStochasticPattern
#include "utils.h"  // for approx_equal
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
        const StageStochasticPattern &stage_stoc_pattern_);

    // copy the lp coefficients from the core file and time file
    // and store the sparsity pattern in the sto file.
    // solver is not initialized
    static StageProblem from_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim,
                                  const smps::SMPSStoch &sto, int stage);

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

    // rhs as in the template
    // fixed part of the rhs
    std::vector<double> rhs_bar;

    // directions of inequalities in current stage
    // should be one of G, L, E
    // (nrows, )
    std::vector<char> inequality_directions;

    // cost coefficients of current stage
    // (nvars_current, )
    std::vector<double> cost_coefficients;

    // position of random elements in the transfer block or RHS
    StageStochasticPattern stage_stoc_pattern;

    // destructor
    ~StageProblem();

    // initialize the solver with the current problem template
    void attach_solver();

    // set the rhs of the solver
    // to the rhs_bar - transfer * z_value - rhs_shift + (dr(omega) - dT(omega) * z)
    // omega should only have the portion of randomness in the current stage
    void apply_scenario_rhs(const std::vector<double> &z_value, const std::vector<double> scenario_omega);

#ifdef UNIT_TEST
    Solver* get_solver() { return solver.get(); }
#endif  // UNIT_TEST

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

    // if the problem has non-trivial bounds
    bool has_non_trivial_bounds;

    // the index of non-trivial bound variables
    std::vector<int> non_trivial_fx_index, non_trivial_lb_index, non_trivial_ub_index;

    // the pointer to the solver environment
    std::unique_ptr<Solver> solver;

};

#endif // PROB_H
