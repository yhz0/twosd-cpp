#ifndef SOLVER_H
#define SOLVER_H
#include "gurobi_c.h"
#include "prob.h"

class Solver
{
    // void set_template(); void set_rhs();
};

// class GurobiSolver : public Solver
// {
//     public:
//     // initialize environment
//     GurobiSolver();

//     // initialize environment binding to another gurobi instance
//     GurobiSolver(GRBenv *_env);

//     // destructor frees model and environment if not null
//     ~GurobiSolver();

//     // load template model from stageproblem
//     void set_template(const StageProblem &stage_problem);

//     private:
//     GRBenv *env;
//     GRBmodel *model;
// };
#endif  // SOLVER_H