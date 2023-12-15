#include "prob.h"

sp::StageProblem::StageProblem(const smps::SMPSCore &core,
                               const smps::SMPSTime &time,
                               const smps::SMPSStoch &pattern,
                               int stage)
{
    size_t total_ncols = core.num_cols, total_nrows = core.num_rows;
    
    // root stage has no previous stage
    if (stage == 0)
        nvars_last = 0;
    else
        nvars_last = time.ncols(stage - 1, core.col_name_map);

    nvars_current = time.ncols(stage, core.col_name_map);
    nrows = time.nrows(stage, core.row_name_map);

    // reserve space for the vectors
    last_stage_var_names.resize(nvars_last);
    current_stage_var_names.resize(nvars_current);
    lb.resize(nvars_current);
    ub.resize(nvars_current);
    cost.resize(nvars_current, 0.0);

    for (size_t i = 0; i < total_ncols; ++i) {
        // scan the column names
        auto current_name = core.col_name_map.get_name(i);
        if (current_name.has_value()) {
            int col_stage, col_index;
            std::tie(col_stage, col_index) = time.get_col_stage(current_name.value(), core.col_name_map);

            if (col_stage == stage - 1) {
                // last stage variable encountered
                last_stage_var_names[col_index] = current_name.value();
            }
            else if (col_stage == stage) {
                // current stage variable encountered
                current_stage_var_names[col_index] = current_name.value();
                lb[col_index] = core.lower_bounds[i];
                ub[col_index] = core.upper_bounds[i];
            }
        }
        else
        {
            // out-of-bound internal error
            throw std::runtime_error("Internal error: column name not found");
        }

        // rows
        current_stage_row_names.resize(nrows);
        for(size_t j = 0; j < total_nrows; ++j) {
            // scan the row names
            auto current_name = core.row_name_map.get_name(j);
            if (current_name.has_value()) {
                int row_stage, row_index;
                std::tie(row_stage, row_index) = time.get_row_stage(current_name.value(), core.row_name_map);

                if (row_stage == stage) {
                    // current stage constraint encountered
                    current_stage_row_names[row_index] = current_name.value();
                    rhs_bar[row_index] = core.rhs_coefficients[j];
                }
            }
            else
            {
                // out-of-bound internal error
                throw std::runtime_error("Internal error: row name not found");
            }
        }

        // TODO: read cost, transfer_block, current_block from core.lpcoefficients
        // iterate through non-zero elements in core
        for (const auto &element : core.lp_coefficients) {
            int cor_row_index, cor_col_index;
            double value;
            std::tie(cor_row_index, cor_col_index, value) = element.data;

            // convert stageness in COR form to stage number and the relative stage index
            int row_index, row_stage, col_index, col_stage;
            std::tie(row_stage, row_index) = time.get_row_stage(
                core.row_name_map.get_name(cor_row_index).value(), core.row_name_map);
            std::tie(col_stage, col_index) = time.get_col_stage(
                core.col_name_map.get_name(cor_col_index).value(), core.col_name_map);

            // check if the element is the cost objective or the current stage constraint
            if (row_stage == -1) {
                if (col_stage == stage) {
                    // cost
                    cost[col_index] = value;
                }
            } else if (row_stage == stage) {
                if (col_stage == stage - 1) {
                    // transfer_block
                    transfer_block.add_element(row_index, col_index, value);
                } else if (col_stage == stage) {
                    // current_block
                    current_block.add_element(row_index, col_index, value);
                }
            }
        }

        // set zero x_base, rhs_shift, cost_shift
        x_base.resize(nvars_current, 0.0);
        rhs_shift.resize(nrows, 0.0);
        cost_shift = 0.0;

        // TODO: construct sparsity pattern from stoc structure

    }

}
