#include "prob.h"
#include "solver.h"
#include <stdexcept>  // For std::runtime_error

StageProblem StageProblem::from_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim, 
                                     const smps::SMPSStoch &sto, int stage)
{
    size_t total_ncols = cor.num_cols, total_nrows = cor.num_rows;
    size_t nvars_last, nvars_current, nrows;
    SparseMatrix<double> transfer_block, current_block;
    std::vector<std::string> last_stage_var_names, current_stage_var_names, current_stage_row_names;
    std::vector<double> lb, ub, x_base, rhs_bar, rhs_shift, cost;
    std::vector<char> inequality_directions;
    double cost_shift = 0.0;

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
    x_base.resize(nvars_current, 0.0);  // Assuming x_base is zero for all current stage variables
    rhs_bar.resize(nrows);
    rhs_shift.resize(nrows, 0.0);  // Assuming rhs_shift is zero initially
    inequality_directions.resize(nrows);

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
                        current_stage_row_names, transfer_block, current_block, lb, ub, x_base, rhs_bar, 
                        rhs_shift, inequality_directions, cost, cost_shift, stage_stoc_pattern);
}

StageProblem::~StageProblem()
{
}
