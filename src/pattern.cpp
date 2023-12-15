#include "pattern.h"

StochasticPattern::StochasticPattern(const std::vector<int> &_stage, const std::vector<int> &_row_index, const std::vector<int> &_col_index, const std::vector<double> &_reference_values) : stage(_stage), row_index(_row_index), col_index(_col_index), reference_values(_reference_values)
{
    // make sure _row_index and _col_index, _reference_values have the same length
    if (row_index.size() != col_index.size() || row_index.size() != reference_values.size()) {
        throw std::runtime_error("StochasticPattern: row_index, col_index, reference_values have different lengths");
    }

    // get the maximum number of stages
    int max_stage = 0;
    for (auto s : stage)
        if (s > max_stage)
            max_stage = s;
    
    // rv_count[i] is the number of random variables in stage i
    rv_count.assign(max_stage + 1, 0);
    for (auto s : stage)
        rv_count[s]++;
    
}

StochasticPattern StochasticPattern::from_smps(const smps::SMPSCore &cor, const smps::SMPSTime &tim, const smps::SMPSStoch &sto)
{
    auto pos = sto.get_positions();

    std::vector<int> _stage;
    std::vector<int> _row_index, _col_index;
    std::vector<double> _reference_values;

    for(const auto &p : pos) {
        int stage;
        double reference_value;

        std::string col_name, row_name;
        std::tie(col_name, row_name) = p;

        // get the stage and index of the row and column
        int row_stage, row_index, col_stage, col_index;
        std::tie(row_stage, row_index) = tim.get_row_stage(row_name, cor.row_name_map);
        std::tie(col_stage, col_index) = tim.get_col_stage(col_name, cor.col_name_map);

        if (row_stage == -1 && col_stage == -1)
        {
            // error, cannot have randomness at (COST, RHS)
            throw std::runtime_error("StochasticPattern::from_smps: randomness at (COST, RHS) is not supported");
        }

        if (row_stage == -1) {
            // cost objective randomness
            // unsupported
            throw std::runtime_error("StochasticPattern::from_smps: randomness at COST is not supported");
        } else if (col_stage == -1) {
            // rhs randomness
            stage = row_stage;
            auto cor_row_index = cor.row_name_map.get_index(row_name);
            if (cor_row_index.has_value())
                reference_value = cor.rhs_coefficients[cor_row_index.value()];
            else
                throw std::runtime_error("StochasticPattern::from_smps: row name " + row_name + " not found in core file");
        }
        else if (col_stage == row_stage - 1)
        {
            // transfer block randomness
            stage = row_stage;
            auto cor_row_index = cor.row_name_map.get_index(row_name),
                    cor_col_index = cor.col_name_map.get_index(col_name);
            if (cor_row_index.has_value() && cor_col_index.has_value())
                reference_value = cor.lp_coefficients.get_element(cor_row_index.value(), cor_col_index.value());
            else
                throw std::runtime_error("StochasticPattern::from_smps: row name " + row_name + " or col name " + col_name + " not found in core file");
        }
        else {
            // unsupported randomness
            throw std::runtime_error("StochasticPattern::from_smps: random position " + row_name + ", " + col_name + " is not supported");
        }

        _stage.push_back(stage);
        _row_index.push_back(row_index);
        _col_index.push_back(col_index);
        _reference_values.push_back(reference_value);
    }

    return StochasticPattern(_stage, _row_index, _col_index, _reference_values);
}
