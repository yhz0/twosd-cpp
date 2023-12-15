#ifndef PROB_H
#define PROB_H

#include "smps.h" // Include smps.h for SMPSCore and SMPSTime

namespace sp {

    class StageProblem {
    public:
        // copy the lp coefficients from the core file and time file
        // and store the sparsity pattern in the sto file.
        StageProblem(const smps::SMPSCore& core, const smps::SMPSTime& time, 
           const smps::SMPSStoch &pattern, int stage);

        // invoke the solver to solve the current lp, writing the primalSolution, dualSolution and optimal value
        virtual void solve_problem(
            const std::vector<double> &prev,
            std::vector<double> &primalSolution,
            std::vector<double> &dualSolution,
            double &optimalValue) = 0;

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

        // cost coefficients of current stage
        // (nvars_current, )
        std::vector<double> cost;

        // shift of current coefficient.
        // c*x = c*(d+x_base) = c*d + c*x_base
        // This term denotes c*x_base part.
        double cost_shift;

    };
}

#endif // PROB_H
