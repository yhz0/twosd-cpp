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
    int numconstrs;
    // Get the number of constraints
    GRBgetintattr(model, "NumConstrs", &numconstrs);

    std::vector<double> rhs(numconstrs);
    // Get the RHS values for all constraints
    GRBgetdblattrarray(model, "RHS", 0, numconstrs, rhs.data());

    return rhs;
}

void Solver::solve()
{
    int error;

    // Optimize model
    error = GRBoptimize(model);
    if (error) { throw std::runtime_error("Solver::solve: error optimizing model"); }

    // Set number of constraints and variables
    GRBgetintattr(model, "NumConstrs", &num_constrs);
    GRBgetintattr(model, "NumVars", &num_vars);
}

std::vector<double> Solver::get_primal_solution() const
{
    std::vector<double> solution(num_vars);
    int error = GRBgetdblattrarray(model, "X", 0, num_vars, solution.data());
    if (error)
    {
        throw std::runtime_error("Solver::get_primal_solution: error getting primal solution");
    }
    return solution;
}

std::vector<double> Solver::get_dual_solution() const
{
    std::vector<double> solution(num_constrs);
    int error = GRBgetdblattrarray(model, "Pi", 0, num_constrs, solution.data());
    if (error)
    {
        throw std::runtime_error("Solver::get_dual_solution: error getting dual solution");
    }

    return solution;
}

std::vector<double> Solver::get_dual_solution(const std::vector<int> &fx_index, const std::vector<int> &lb_index, const std::vector<int> &ub_index) const
{
    std::vector<double> solution(num_constrs + fx_index.size() + lb_index.size() + ub_index.size());

    // constraint block duals
    int error = GRBgetdblattrarray(model, "Pi", 0, num_constrs, solution.data());
    if (error)
    {
        throw std::runtime_error("Solver::get_dual_solution: error getting dual solution");
    }

    // the position of the variable RC
    int offset = num_constrs;

    // fixed variables duals
    for (size_t i = 0; i < fx_index.size(); ++i)
    {
        error = GRBgetdblattrelement(model, "RC", fx_index[i], &solution[offset + i]);
        if (error)
        {
            throw std::runtime_error("Solver::get_dual_solution: error getting RC for fixed variables");
        }
    }

    offset += fx_index.size();

    // lb duals
    for(size_t i = 0; i < lb_index.size(); ++i)
    {
        double x, lb;
        // query X and lower bound to check if the bound is achieved
        error = GRBgetdblattrelement(model, "X", lb_index[i], &x);
        if (error)
        {
            throw std::runtime_error("Solver::get_dual_solution: error getting X for lb");
        }

        // get lower bound value
        error = GRBgetdblattrelement(model, "LB", lb_index[i], &lb);

        if (error) {
            throw std::runtime_error("Solver::get_dual_solution: error getting LB for lb");
        }

        if (approx_equal(x, lb))
        {
            error = GRBgetdblattrelement(model, "RC", lb_index[i], &solution[offset + i]);
            if (error)
            {
                throw std::runtime_error("Solver::get_dual_solution: error getting RC for lb");
            }
        }
        else
        {
            solution[offset + i] = 0.0;
        }
    }

    offset += lb_index.size();

    // ub duals
    for(size_t i = 0; i < ub_index.size(); ++i)
    {
        double x, ub;
        // query X and upper bound to check if the bound is achieved
        error = GRBgetdblattrelement(model, "X", ub_index[i], &x);
        if (error)
        {
            throw std::runtime_error("Solver::get_dual_solution: error getting X for ub");
        }

        // get upper bound value
        error = GRBgetdblattrelement(model, "UB", ub_index[i], &ub);

        if (error) {
            throw std::runtime_error("Solver::get_dual_solution: error getting UB for ub");
        }

        if (approx_equal(x, ub))
        {
            error = GRBgetdblattrelement(model, "RC", ub_index[i], &solution[offset + i]);
            if (error)
            {
                throw std::runtime_error("Solver::get_dual_solution: error getting RC for ub");
            }
        }
        else
        {
            solution[offset + i] = 0.0;
        }
    }

    return solution;
}

Solver::~Solver()
{
    if (model != nullptr)
        GRBfreemodel(model);
    if (env != nullptr)
        GRBfreeenv(env);
}

