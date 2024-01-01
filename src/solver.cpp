#include "solver.h"
#include <stdexcept> // For std::runtime_error
#include <vector>

Solver::Solver() : env(nullptr), model(nullptr) {}

Solver::Solver(GRBenv *env, GRBmodel *model) : env(env), model(model) {}

Solver Solver::from_template(StageProblem &stage_problem)
{
    GRBenv *env;
    GRBmodel *model;
    
    int error = 0;
    // Create environment
    error = GRBloadenv(&env, nullptr);
    if (error)
    {
        throw std::runtime_error("Solver::from_template: error creating environment");
    }

    // Create model
    std::vector<double> cost(stage_problem.cost_coefficients),
                        lb(stage_problem.lb),
                        ub(stage_problem.ub);
    error = GRBnewmodel(env, &model,
                        "",
                        stage_problem.nvars_current,
                        cost.data(),
                        lb.data(),
                        ub.data(),
                        nullptr,
                        nullptr);
    if (error)
    {
        throw std::runtime_error("Solver::from_template: error creating model");
    }

    // Add constraints block
    SparseMatrixCSR A(stage_problem.current_block);
    std::vector<int> cbeg(A.getRowBegin()),
                        cind(A.getColumnIndex());
    std::vector<double> cval(A.getValues());

    // generate sense that is compatible with gurobi
    std::vector<char> sense;
    sense.resize(stage_problem.nrows);
    for(size_t i = 0; i < stage_problem.nrows; ++i){
        if (stage_problem.inequality_directions[i] == 'L')
            sense[i] = GRB_LESS_EQUAL;
        else if (stage_problem.inequality_directions[i] == 'G')
            sense[i] = GRB_GREATER_EQUAL;
        else if (stage_problem.inequality_directions[i] == 'E')
            sense[i] = GRB_EQUAL;
        else
            throw std::runtime_error("Solver::from_template: invalid inequality direction");
    }

    std::vector<double> rhs_bar(stage_problem.rhs_bar);

    error = GRBaddconstrs(model, stage_problem.nrows,
                  stage_problem.current_block.nnz(),
                  cbeg.data(), cind.data(), cval.data(),
                  sense.data(),
                  rhs_bar.data(),
                  nullptr);
    if (error)
    {
        throw std::runtime_error("Solver::from_template: error adding constraints");
    }

    // copy constraint names
    for (size_t i = 0; i < stage_problem.nrows; ++i)
    {
        error = GRBsetstrattrelement(model, "ConstrName", i, stage_problem.current_stage_row_names[i].c_str());
        if (error)
        {
            throw std::runtime_error("Solver::from_template: error setting constraint names");
        }
    }

    // copy variable names
    for (size_t i = 0; i < stage_problem.nvars_current; ++i)
    {
        error = GRBsetstrattrelement(model, "VarName", i, stage_problem.current_stage_var_names[i].c_str());
        if (error)
        {
            throw std::runtime_error("Solver::from_template: error setting variable names");
        }
    }

    // update model
    error = GRBupdatemodel(model);

    return Solver(env, model);
}

void Solver::write_model(const char *filename) const
{
    if (env == nullptr)
    {
        throw std::runtime_error("Solver::write_model: environment is null");
    }

    if (model == nullptr)
    {
        throw std::runtime_error("Solver::write_model: model is null");
    }

    int error = GRBwrite(model, filename);
    if (error)
    {
        throw std::runtime_error("Solver::write_model: error writing model");
    }
}

void Solver::set_rhs(const std::vector<double> &rhs)
{
    int error = GRBsetdblattrarray(model, "RHS", 0, rhs.size(), const_cast<double *>(rhs.data()));
    if (error)
    {
        throw std::runtime_error("Solver::change_rhs: error changing RHS.");
    }

    error = GRBupdatemodel(model);
    if (error)
    {
        throw std::runtime_error("Solver::change_rhs: error updating model.");
    }
}

std::vector<double> Solver::get_rhs() const
{
    int       numconstrs;
    // Get the number of constraints
    GRBgetintattr(model, "NumConstrs", &numconstrs);

    std::vector<double> rhs(numconstrs);
    // Get the RHS values for all constraints
    GRBgetdblattrarray(model, "RHS", 0, numconstrs, rhs.data());

    return rhs;
}

Solver::~Solver()
{
    if (model != nullptr)
        GRBfreemodel(model);
    if (env != nullptr)
        GRBfreeenv(env);
}

