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

    // Move constructor
    Solver(Solver&& other) noexcept
        : env(other.env), model(other.model) {
        other.env = nullptr;  // The original object no longer owns the resource
        other.model = nullptr;
    }

    // Delete copy constructor
    Solver &operator=(const Solver &) = delete;

    // load template model from stage problem
    // the template includes the current stage variables and the objective function
    // this does not set the RHS part of the model
    static Solver from_template(StageProblem &stage_problem);

    // write model to LP file for debug purposes
    void write_model(const char *filename) const;

    // change the RHS part of the model
    void set_rhs(const std::vector<double> &rhs);

    // get the full RHS
    std::vector<double> get_rhs() const;

    // destructor frees model and environment if not null
    ~Solver();

    private:
    GRBenv *env;
    GRBmodel *model;
};

#endif  // SOLVER_H