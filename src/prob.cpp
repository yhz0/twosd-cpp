#include "prob.h"
#include "solver.h"
#include <stdexcept> // For std::runtime_error

StageProblem::StageProblem(size_t nvars_last_, size_t nvars_current_, size_t nrows_, const std::vector<std::string> &last_stage_var_names_, const std::vector<std::string> &current_stage_var_names_, const std::vector<std::string> &current_stage_row_names_, const SparseMatrix<double> &transfer_block_, const SparseMatrix<double> &current_block_, const std::vector<double> &lb_, const std::vector<double> &ub_, const std::vector<double> &rhs_bar_, const std::vector<char> &inequality_directions_, const std::vector<double> &cost_coefficients_, const StageStochasticPattern &stage_stoc_pattern_)
    : nvars_last(nvars_last_), nvars_current(nvars_current_), nrows(nrows_),
      last_stage_var_names(last_stage_var_names_), current_stage_var_names(current_stage_var_names_), current_stage_row_names(current_stage_row_names_),
      transfer_block(transfer_block_), current_block(current_block_),
      lb(lb_), ub(ub_), rhs_bar(rhs_bar_),
      inequality_directions(inequality_directions_),
      cost_coefficients(cost_coefficients_),
      stage_stoc_pattern(stage_stoc_pattern_),
      // private member initializers
      shift_x_base(false),
      x_base(nvars_current_, 0.0), // set zero for shifts
      rhs_shift(nrows_, 0.0),
      cost_shift(0.0),
      has_non_trivial_bounds(false),
      non_trivial_fx_index(),
      non_trivial_lb_index(),
      non_trivial_ub_index(),
      solver(nullptr)
{
    // check if the problem has non-trivial bounds
    for (size_t i = 0; i < nvars_current; ++i)
    {
        // fix bound if ub==lb and non zero
        if (ub[i] == lb[i] && !approx_equal(lb[i], 0.0))
            non_trivial_fx_index.push_back(i);

        if (ub[i] != std::numeric_limits<double>::infinity() && !approx_equal(ub[i], 0.0))
            non_trivial_ub_index.push_back(i);
        else if (lb[i] != -std::numeric_limits<double>::infinity() && !approx_equal(lb[i], 0.0))
            non_trivial_lb_index.push_back(i);
    }

    if (non_trivial_fx_index.size() + non_trivial_ub_index.size() + non_trivial_lb_index.size() > 0)
    {
        has_non_trivial_bounds = true;
    }
}

StageProblem StageProblem::from_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim,
                                     const smps::SMPSStoch &sto, int stage)
{
    size_t total_ncols = cor.num_cols, total_nrows = cor.num_rows;
    size_t nvars_last, nvars_current, nrows;
    SparseMatrix<double> transfer_block, current_block;
    std::vector<std::string> last_stage_var_names, current_stage_var_names, current_stage_row_names;
    std::vector<double> lb, ub, x_base, rhs_bar, rhs_shift, cost;
    std::vector<char> inequality_directions;

    // Initialize variables based on stage
    nvars_last = (stage == 0) ? 0 : tim.ncols(stage - 1, cor.col_name_map);
    nvars_current = tim.ncols(stage, cor.col_name_map);
    nrows = tim.nrows(stage, cor.row_name_map);

    // Reserve space for vectors
    last_stage_var_names.resize(nvars_last);
    current_stage_var_names.resize(nvars_current);
    current_stage_row_names.resize(nrows);
    lb.resize(nvars_current);
    ub.resize(nvars_current);
    cost.resize(nvars_current, 0.0);
    x_base.resize(nvars_current, 0.0); // Assuming x_base is zero for all current stage variables
    rhs_bar.resize(nrows);
    rhs_shift.resize(nrows, 0.0); // Assuming rhs_shift is zero initially
    inequality_directions.resize(nrows);

    // Set matrix size
    transfer_block.resize(nrows, nvars_last);
    current_block.resize(nrows, nvars_current);

    // Process columns
    for (size_t i = 0; i < total_ncols; ++i)
    {
        // Scan the column names and process them
        auto current_name = cor.col_name_map.get_name(i);
        if (!current_name.has_value())
        {
            throw std::runtime_error("StageProblem::from_smps: column name not found - " + std::to_string(i));
        }

        int col_stage, col_index;
        std::tie(col_stage, col_index) = tim.get_col_stage(current_name.value(), cor.col_name_map);

        if (col_stage == stage - 1)
        {
            last_stage_var_names[col_index] = current_name.value();
        }
        else if (col_stage == stage)
        {
            current_stage_var_names[col_index] = current_name.value();
            lb[col_index] = cor.lower_bounds[i];
            ub[col_index] = cor.upper_bounds[i];
        }
    }

    // Process rows
    for (size_t j = 0; j < total_nrows; ++j)
    {
        // Scan the row names and process them
        auto current_name = cor.row_name_map.get_name(j);
        if (!current_name.has_value())
        {
            throw std::runtime_error("StageProblem::from_smps: row name not found");
        }

        int row_stage, row_index;
        std::tie(row_stage, row_index) = tim.get_row_stage(current_name.value(), cor.row_name_map);

        if (row_stage == stage)
        {
            current_stage_row_names[row_index] = current_name.value();
            rhs_bar[row_index] = cor.rhs_coefficients[j];
            inequality_directions[row_index] = cor.inequality_directions[j];
        }
    }

    // Process LP coefficients
    for (const auto &element : cor.lp_coefficients)
    {
        int cor_row_index, cor_col_index;
        double value;

        cor_row_index = element.row;
        cor_col_index = element.col;
        value = element.val;

        // Convert stageness in COR form to stage number and the relative stage index
        int row_index, row_stage, col_index, col_stage;
        std::tie(row_stage, row_index) = tim.get_row_stage(
            cor.row_name_map.get_name(cor_row_index).value(), cor.row_name_map);
        std::tie(col_stage, col_index) = tim.get_col_stage(
            cor.col_name_map.get_name(cor_col_index).value(), cor.col_name_map);

        // Check if the element is the cost objective or the current stage constraint
        if (row_stage == -1 && col_stage == stage)
        {
            // Cost
            cost[col_index] = value;
        }
        else if (row_stage == stage)
        {
            if (col_stage == stage - 1)
            {
                // Transfer block
                transfer_block.add_element(row_index, col_index, value);
            }
            else if (col_stage == stage)
            {
                // Current block
                current_block.add_element(row_index, col_index, value);
            }
        }
    }

    // Read the stochastic pattern
    StageStochasticPattern stage_stoc_pattern = StochasticPattern::from_smps(cor, tim, sto).filter_by_stage(stage);

    // Create and return StageProblem instance
    return StageProblem(nvars_last, nvars_current, nrows, last_stage_var_names, current_stage_var_names,
                        current_stage_row_names, transfer_block, current_block, lb, ub, rhs_bar,
                        inequality_directions, cost, stage_stoc_pattern);
}

StageProblem::~StageProblem()
{
}

void StageProblem::attach_solver()
{
    Solver solver_instance = Solver::from_template(*this);
    solver = std::make_unique<Solver>(std::move(solver_instance));
}

void StageProblem::apply_scenario_rhs(const std::vector<double> &z_value, const std::vector<double> scenario_omega)
{
    // new_rhs = rhs_bar - transfer * z_value - rhs_shift + (dr(omega) - dT(omega) * z)
    std::vector<double> new_rhs(rhs_bar);

    // Apply transfer block
    if (transfer_block.nnz() > 0)
        transfer_block.subtract_multiply_with_vector(z_value, new_rhs);

    // Apply RHS shift
    if (shift_x_base)
        for (size_t i = 0; i < rhs_shift.size(); ++i)
            new_rhs[i] -= rhs_shift[i];

    // Apply stochastic pattern
    for (size_t i = 0; i < stage_stoc_pattern.rv_count; ++i)
    {
        int row = stage_stoc_pattern.row_index[i], col = stage_stoc_pattern.col_index[i];
        double ref_value = stage_stoc_pattern.reference_values[i],
               current_value = scenario_omega[i];

        if (col == -1)
        {
            // RHS
            new_rhs[row] += current_value - ref_value;
        }
        else if (row == -1)
        {
            // cost
            throw std::runtime_error("StageProblem::apply_scenario_rhs: randomness in cost is not supported");
        }
        else
        {
            // transfer block
            new_rhs[row] -= (current_value - ref_value) * z_value[col];
        }
    }

    // Set the new RHS
    solver->set_rhs(new_rhs);
}
