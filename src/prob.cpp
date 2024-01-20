#include "prob.h"
#include <stdexcept> // For std::runtime_error

#if UNIT_TEST
#include <iostream>
#endif

StageProblem::StageProblem(const smps::SMPSCore &cor, const smps::SMPSTime &tim, const smps::SMPSStoch &sto, int stage): env(nullptr), model(nullptr)
{
    read_smps(cor, tim, sto, stage);
}

StageProblem::StageProblem(const StageProblem &other)
    : nvars_last(other.nvars_last), nvars_current(other.nvars_current), nrows(other.nrows),
      last_stage_var_names(other.last_stage_var_names), current_stage_var_names(other.current_stage_var_names), current_stage_row_names(other.current_stage_row_names),
      transfer_block(other.transfer_block), current_block(other.current_block),
      lb(other.lb), ub(other.ub), rhs_bar(other.rhs_bar),
      inequality_directions(other.inequality_directions),
      cost_coefficients(other.cost_coefficients),
      stage_stoc_pattern(other.stage_stoc_pattern),
      // private member initializers
      shift_x_base(other.shift_x_base),
      x_base(other.x_base),
      rhs_shift(other.rhs_shift),
      cost_shift(other.cost_shift),
      has_non_trivial_bounds(other.has_non_trivial_bounds),
      non_trivial_fx_index(other.non_trivial_fx_index),
      non_trivial_lb_index(other.non_trivial_lb_index),
      non_trivial_ub_index(other.non_trivial_ub_index),
      env(nullptr), model(nullptr) // solver is not copied
{
}

StageProblem::StageProblem(StageProblem &&other) noexcept
    : nvars_last(other.nvars_last), nvars_current(other.nvars_current), nrows(other.nrows),
      last_stage_var_names(std::move(other.last_stage_var_names)), current_stage_var_names(std::move(other.current_stage_var_names)), current_stage_row_names(std::move(other.current_stage_row_names)),
      transfer_block(std::move(other.transfer_block)), current_block(std::move(other.current_block)),
      lb(std::move(other.lb)), ub(std::move(other.ub)), rhs_bar(std::move(other.rhs_bar)),
      inequality_directions(std::move(other.inequality_directions)),
      cost_coefficients(std::move(other.cost_coefficients)),
      stage_stoc_pattern(std::move(other.stage_stoc_pattern)),
      // private member initializers
      shift_x_base(other.shift_x_base),
      x_base(std::move(other.x_base)),
      rhs_shift(std::move(other.rhs_shift)),
      cost_shift(other.cost_shift),
      has_non_trivial_bounds(other.has_non_trivial_bounds),
      non_trivial_fx_index(std::move(other.non_trivial_fx_index)),
      non_trivial_lb_index(std::move(other.non_trivial_lb_index)),
      non_trivial_ub_index(std::move(other.non_trivial_ub_index)),
      env(other.env), model(other.model) // solver is moved
{
    other.env = nullptr;
    other.model = nullptr;
}

StageProblem &StageProblem::operator=(StageProblem &&other) noexcept
{
    if (this != &other)
    {
        nvars_last = other.nvars_last;
        nvars_current = other.nvars_current;
        nrows = other.nrows;
        last_stage_var_names = std::move(other.last_stage_var_names);
        current_stage_var_names = std::move(other.current_stage_var_names);
        current_stage_row_names = std::move(other.current_stage_row_names);
        transfer_block = std::move(other.transfer_block);
        current_block = std::move(other.current_block);
        lb = std::move(other.lb);
        ub = std::move(other.ub);
        rhs_bar = std::move(other.rhs_bar);
        inequality_directions = std::move(other.inequality_directions);
        cost_coefficients = std::move(other.cost_coefficients);
        stage_stoc_pattern = std::move(other.stage_stoc_pattern);
        // private member initializers
        shift_x_base = other.shift_x_base;
        x_base = std::move(other.x_base);
        rhs_shift = std::move(other.rhs_shift);
        cost_shift = other.cost_shift;
        has_non_trivial_bounds = other.has_non_trivial_bounds;
        non_trivial_fx_index = std::move(other.non_trivial_fx_index);
        non_trivial_lb_index = std::move(other.non_trivial_lb_index);
        non_trivial_ub_index = std::move(other.non_trivial_ub_index);
        
        // Move the solver
        env = other.env;
        model = other.model;
        other.env = nullptr;
        other.model = nullptr;
    }
    return *this;
}

void StageProblem::attach_solver()
{
    int error = 0;
    if (env == nullptr)
    {
        error = GRBloadenv(&env, nullptr);
        if (error)
        {
            throw std::runtime_error("StageProblem::attach_solver: error creating environment");
        }
    }

    // empty model if not null
    if (model != nullptr)
        GRBfreemodel(model);
    
    // Create model
    error = GRBnewmodel(env, &model,
                        "",
                        nvars_current,
                        cost_coefficients.data(),
                        lb.data(),
                        ub.data(),
                        nullptr,
                        nullptr);
    if (error)
    {
        throw std::runtime_error("StageProblem::attach_solver: error creating model");
    }

    // Add constraints block
    SparseMatrixCSR A(current_block);
    std::vector<int> cbeg(A.getRowBegin()),
        cind(A.getColumnIndex());
    std::vector<double> cval(A.getValues());

    // generate sense that is compatible with gurobi
    std::vector<char> sense;
    sense.resize(nrows);
    for (size_t i = 0; i < nrows; ++i)
    {
        if (inequality_directions[i] == 'L')
            sense[i] = GRB_LESS_EQUAL;
        else if (inequality_directions[i] == 'G')
            sense[i] = GRB_GREATER_EQUAL;
        else if (inequality_directions[i] == 'E')
            sense[i] = GRB_EQUAL;
        else
            throw std::runtime_error("StageProblem::attach_solver: invalid inequality direction");
    }

    error = GRBaddconstrs(model, nrows,
                          current_block.nnz(),
                          cbeg.data(), cind.data(), cval.data(),
                          sense.data(),
                          rhs_bar.data(),
                          nullptr);
    if (error)
    {
        throw std::runtime_error("StageProblem::attach_solver: error adding constraints");
    }

    // set constraint names
    for(size_t i = 0; i < nrows; ++i){
        error = GRBsetstrattrelement(model, "ConstrName", i, current_stage_row_names[i].c_str());
        if (error)
        {
            throw std::runtime_error("StageProblem::attach_solver: error setting constraint name");
        }
    }

    // set variable names
    for(size_t i = 0; i < nvars_current; ++i){
        error = GRBsetstrattrelement(model, "VarName", i, current_stage_var_names[i].c_str());
        if (error)
        {
            throw std::runtime_error("StageProblem::attach_solver: error setting variable name");
        }
    }

    error = GRBupdatemodel(model);
    if (error)
    {
        throw std::runtime_error("StageProblem::attach_solver: error updating model");
    }
}

void StageProblem::update_solver_with_scenario(const std::vector<double> &z_value, const std::vector<double> &scenario_omega)
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
            throw std::runtime_error("StageProblem::update_solver_with_scenario: randomness in cost is not supported");
        }
        else
        {
            // transfer block
            new_rhs[row] -= (current_value - ref_value) * z_value[col];
        }
    }

    // Set the new RHS
    int error = GRBsetdblattrarray(model, GRB_DBL_ATTR_RHS, 0, nrows, new_rhs.data());
    if (error)
    {
        throw std::runtime_error("StageProblem::update_solver_with_scenario: error setting RHS");
    }

    // update bounds if shifted
    if (shift_x_base)
        update_solver_bounds();

    error = GRBupdatemodel(model);
    if (error)
    {
        throw std::runtime_error("StageProblem::update_solver_with_scenario: error updating model");
    }
}

void StageProblem::update_solver_root_stage()
{
    // new_rhs = rhs_bar - rhs_shift
    std::vector<double> new_rhs(rhs_bar);

    if (shift_x_base)
        for (size_t i = 0; i < rhs_shift.size(); ++i)
            new_rhs[i] -= rhs_shift[i];
    
    // Set the new RHS
    int error = GRBsetdblattrarray(model, GRB_DBL_ATTR_RHS, 0, nrows, new_rhs.data());
    if (error)
    {
        throw std::runtime_error("StageProblem::update_solver_root_stage: error setting RHS");
    }

    // update the bounds if shifted
    if (shift_x_base)
        update_solver_bounds();

    error = GRBupdatemodel(model);
    if (error)
    {
        throw std::runtime_error("StageProblem::update_solver_root_stage: error updating model");
    }
}

void StageProblem::set_x_base(const std::vector<double> &x_base_)
{
    if (x_base_.size() != nvars_current)
    {
        throw std::runtime_error("StageProblem::set_x_base: x_base_ has wrong size");
    }
    x_base = x_base_;
    shift_x_base = true;

    // update rhs_shift and cost_shift
    update_rhs_shift();
    update_cost_shift();
}

void StageProblem::unset_x_base()
{
    shift_x_base = false;

    // update rhs_shift and cost_shift, this will set them to zero
    update_rhs_shift();
    update_cost_shift();

    if (is_solver_attached())
    {
        update_solver_bounds();  
    }
}

double StageProblem::get_cost_shift() const
{
    if (shift_x_base)
        return cost_shift;
    else
        return 0.0;
}

StageProblem::~StageProblem()
{
    if (model != nullptr)
        GRBfreemodel(model);
    if (env != nullptr)
        GRBfreeenv(env);
}

void StageProblem::update_rhs_shift()
{
    // zero rhs_shift
    std::fill(rhs_shift.begin(), rhs_shift.end(), 0.0);
    if (!shift_x_base)
        return;
    // then add current_block * x_base
    current_block.multiply_with_vector(x_base, rhs_shift);
}

void StageProblem::update_cost_shift()
{
    cost_shift = 0.0;
    if (!shift_x_base)
        return;
    // set cost_shift to c*x_base
    for (size_t i = 0; i < nvars_current; ++i)
    {
        cost_shift += cost_coefficients[i] * x_base[i];
    }
}

void StageProblem::update_solver_bounds()
{
    // lb - x_base <= d <= ub - x_base
    // update lb and ub if non-trivial
    for (size_t i = 0; i < nvars_current; ++i)
    {
        // lower bound
        if (lb[i] != -std::numeric_limits<double>::infinity())
        {
            int error = GRBsetdblattrelement(model, "LB", i, lb[i] - x_base[i]);
            if (error)
            {
                throw std::runtime_error("StageProblem::update_bounds: error setting lower bound");
            }
        }

        // upper bound
        if (ub[i] != std::numeric_limits<double>::infinity())
        {
            int error = GRBsetdblattrelement(model, "UB", i, ub[i] - x_base[i]);
            if (error)
            {
                throw std::runtime_error("StageProblem::update_bounds: error setting upper bound");
            }
        }
    }
}

void StageProblem::read_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim, const smps::SMPSStoch &sto, int stage)
{
    size_t total_ncols = cor.num_cols, total_nrows = cor.num_rows;

    // Initialize variables based on stage
    nvars_last = (stage == 0) ? 0 : tim.ncols(stage - 1, cor.col_name_map);
    nvars_current = tim.ncols(stage, cor.col_name_map);
    nrows = tim.nrows(stage, cor.row_name_map);

    // Reserve space for vectors
    last_stage_var_names.clear();
    last_stage_var_names.resize(nvars_last);

    current_stage_var_names.clear();
    current_stage_var_names.resize(nvars_current);

    current_stage_row_names.clear();
    current_stage_row_names.resize(nrows);

    lb.assign(nvars_current, 0.0);
    ub.assign(nvars_current, 0.0);
    cost_coefficients.assign(nvars_current, 0.0);
    x_base.assign(nvars_current, 0.0);
    rhs_bar.assign(nrows, 0.0);
    rhs_shift.assign(nrows, 0.0); // Assuming rhs_shift is zero initially
    inequality_directions.assign(nrows, '\0');

    // Set matrix size
    transfer_block.clear();
    transfer_block.resize(nrows, nvars_last);

    current_block.clear();
    current_block.resize(nrows, nvars_current);

    // Process columns
    for (size_t i = 0; i < total_ncols; ++i)
    {
        // Scan the column names and process them
        auto current_name = cor.col_name_map.get_name(i);
        if (!current_name.has_value())
        {
            throw std::runtime_error("StageProblem::read_smps: column name not found - " + std::to_string(i));
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
            throw std::runtime_error("StageProblem::read_smps: row name not found");
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
            cost_coefficients[col_index] = value;
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
    stage_stoc_pattern = StochasticPattern::from_smps(cor, tim, sto).filter_by_stage(stage);

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
    } else {
        has_non_trivial_bounds = false;
    }

    cost_shift = 0.0;
}

bool StageProblem::is_solver_attached() const
{
    return model != nullptr;
}

void StageProblem::remove_quadratic_term()
{
    int error = GRBdelq(model);
    if (error)
    {
        throw std::runtime_error("StageProblem::remove_quadratic_term: error removing quadratic term");
    }

    error = GRBupdatemodel(model);
}

void StageProblem::add_quadratic_term(double scale)
{
    std::vector<int> qrow(nvars_current);
    std::vector<int> qcol(nvars_current);
    std::vector<double> qval(nvars_current);

    for (size_t i = 0; i < nvars_current; ++i) {
        qrow[i] = static_cast<int>(i);
        qcol[i] = static_cast<int>(i);
        qval[i] = scale;
    }

    int error = GRBaddqpterms(model, static_cast<int>(nvars_current), qrow.data(), qcol.data(), qval.data());

    if (error)
    {
        throw std::runtime_error("StageProblem::add_quadratic_term: error adding quadratic term");
    }

    error = GRBupdatemodel(model);
}
