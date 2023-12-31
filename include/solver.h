#ifndef SOLVER_H
#define SOLVER_H
#include "gurobi_c.h"
#include "prob.h"

class StageProblem;

class Solver
{
    // void set_template(); void set_rhs();
    public:

    Solver();
    Solver(GRBenv *env, GRBmodel *model);

    // load template model from stage problem
    // the template includes the current stage variables and the objective function
    // this does not set the RHS part of the model
    static Solver from_template(StageProblem &stage_problem);

    // write model to LP file for debug purposes
    void write_model(const char *filename);

    // destructor frees model and environment if not null
    ~Solver();

    private:
    GRBenv *env;
    GRBmodel *model;
};

#endif  // SOLVER_H