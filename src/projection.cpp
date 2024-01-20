#include "projection.h"
#include "utils.h"

void StageProjectionProblem::attach_solver()
{
    StageProblem::attach_solver();

    // remove linear terms from the objective
    remove_linear_terms();

    // remove quadratic terms from the objective
    remove_quadratic_term();

    // add the quadratic objective
    add_quadratic_term(0.5);
}

std::optional<std::vector<double>> StageProjectionProblem::solve(const std::vector<double> &x0)
{
    if (is_feasible(x0))
    {
        return std::nullopt;
    }

    // use set_x_base to set the base point
    set_x_base(x0);
    update_solver_root_stage();

    // call the solver
    int error = GRBoptimize(model);
    if (error)
    {
        throw std::runtime_error("StageProjectionProblem::solve: Gurobi error code " + std::to_string(error) + " when optimizing.");
    }

    // get the solution
    std::vector<double> solution(nvars_current, 0.0);
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, nvars_current, solution.data());

    if (error)
    {
        throw std::runtime_error("StageProjectionProblem::solve: Gurobi error code " + std::to_string(error) + " when getting solution.");
    }

    return std::make_optional(solution);
}

void StageProjectionProblem::remove_linear_terms()
{
    std::vector<double> obj(nvars_current, 0.0);
    // set the linear objective to 0
    int error = GRBsetdblattrarray(model, GRB_DBL_ATTR_OBJ, 0, nvars_current, obj.data());
    if (error)
    {
        throw std::runtime_error("StageProjectionProblem::remove_linear_terms: Gurobi error code " + std::to_string(error) + " when setting linear objective to 0.");
    }
}

bool StageProjectionProblem::is_feasible(const std::vector<double> &x0)
{
    // first look at the bounds
    for (size_t i = 0; i < nvars_current; i++)
    {
        if (x0[i] < lb[i] || x0[i] > ub[i])
        {
            return false;
        }
    }

    // multiply the constraint matrix with x0
    std::vector<double> result(nrows, 0.0);
    current_block.multiply_with_vector(x0, result);

    // check the inequality constraints
    for (size_t i = 0; i < nrows; i++)
    {
        if (inequality_directions[i] == 'G' && result[i] < rhs_bar[i])
        {
            return false;
        }
        else if (inequality_directions[i] == 'L' && result[i] > rhs_bar[i])
        {
            return false;
        }
        else if (inequality_directions[i] == 'E' && !approx_equal(result[i], rhs_bar[i]))
        {
            return false;
        }
    }

    return true;
}
