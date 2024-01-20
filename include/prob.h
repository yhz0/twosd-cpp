#ifndef PROB_H
#define PROB_H

#include "smps.h"
#include "pattern.h"    // for StageStochasticPattern
#include "utils.h"  // for approx_equal
#include "gurobi_c.h"

class StageProblem
{
public:
    // copy the lp coefficients from the core file and time file
    // and store the sparsity pattern in the sto file.
    // solver is not initialized
    StageProblem(const smps::SMPSCore &cor, const smps::SMPSTime &tim,
                                  const smps::SMPSStoch &sto, int stage);

    // copy constructor
    // will not copy the solver
    StageProblem(const StageProblem& other);

    // Declare the move constructor and move assignment operator
    StageProblem(StageProblem&& other) noexcept;
    StageProblem& operator=(StageProblem&& other) noexcept;

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

    // initialize the solver with the current problem template
    virtual void attach_solver();

    // update the solver with the current problem template and the specified x_base
    // change rhs to the rhs_bar - transfer * z_value - rhs_shift + (dr(omega) - dT(omega) * z)
    // and bounds to the shifted bounds
    // omega should only have the portion of randomness in the current stage
    void update_solver_with_scenario(const std::vector<double> &z_value, const std::vector<double> &scenario_omega);

    // update the solver with the current problem template and the specified x_base
    // change rhs to the rhs_bar - rhs_shift
    // and bounds to the shifted bounds if necessary
    void update_solver_root_stage();

    // struct for storing the solution
    struct Solution {
        // the optimal value
        double obj_value;
        // the optimal solution
        std::vector<double> solution;
        // the dual solution
        std::vector<double> dual_solution;        
    };

    // solve the problem
    Solution solve_problem(bool require_dual_solution = false);

    // set x_base to the specified value and update cost_shift and rhs_shift
    void set_x_base(const std::vector<double> &x_base_);

    // unset x_base and update cost_shift and rhs_shift
    void unset_x_base();

    // return the cost shift
    double get_cost_shift() const;

    // add quadratic term to the objective, keeping the linear term
    // the quadratic term is scale * x^2
    void add_quadratic_term(double scale);

    // remove all quadratic term, keeping the linear term
    void remove_quadratic_term();

    // get the dimension of the dual vector
    size_t get_dual_dimension() const;

    ~StageProblem();

#ifdef UNIT_TEST
    // for unit test, expose the gurobi environment and model
    GRBenv* get_env() const { return env; }
    GRBmodel* get_model() const { return model; }
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

    // update the rhs shift
    // if x_base is set, set rhs_shift to A*x_base
    // otherwise, set rhs_shift to zero vector
    void update_rhs_shift();

    // update the cost shift
    // if x_base is set, set cost_shift to c*x_base
    // otherwise, set cost_shift to 0
    void update_cost_shift();

    // update the bounds to the solver with the shifted bounds
    void update_solver_bounds();

    // get primal solution from the solver
    std::vector<double> get_primal_solution() const;

    // get dual solution from the solver
    std::vector<double> get_dual_solution() const;

    // get the objective value from the solver
    double get_obj_value() const;

    // read from smps files
    void read_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim,
                   const smps::SMPSStoch &sto, int stage);

    protected:

    bool is_solver_attached() const;

    // the pointer to the solver environment
    GRBenv *env;
    GRBmodel *model;

};

#endif // PROB_H
