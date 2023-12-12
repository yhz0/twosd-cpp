#ifndef PROB_H
#define PROB_H

#include "smps.h" // Include smps.h for SMPSCore and SMPSTime

namespace sp {

    class StageProblem {
    public:
        // create a solver environment and set up an LP/QP problem.
        virtual void initialize_solver() = 0;

        // build a template lp from the core file and time file.
        // This sets up a problem with variables of the last stage and current stage,
        // and add all constraints in the current stage.
        virtual void build_problem(const smps::SMPSCore& core, const smps::SMPSTime& time, int stage) = 0;

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
    };
}

#endif // PROB_H
