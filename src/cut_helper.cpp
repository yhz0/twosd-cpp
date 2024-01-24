#include "cut_helper.h"

Cut CutHelper::get_static_part(const StageProblem &prob, const std::vector<double> &pi)
{
    // intercept
    double alpha = 0.0;
    for (size_t i = 0; i < prob.nrows; ++i)
        alpha += prob.rhs_bar[i] * pi[i];

    // non trivial bounds
    // pi is ordered as [pi, pi_fx, pi_lb, pi_ub]
    size_t pos = prob.nrows;
    if (prob.has_non_trivial_bounds)
    {
        // fix bound means lb = ub so we only need to add one of them
        for (size_t i = 0; i < prob.non_trivial_fx_index.size(); ++i)
            alpha += prob.ub[i] * pi[pos++];
        for (size_t i = 0; i < prob.non_trivial_lb_index.size(); ++i)
            alpha += prob.lb[i] * pi[pos++];
        for (size_t i = 0; i < prob.non_trivial_ub_index.size(); ++i)
            alpha += prob.ub[i] * pi[pos++];   
    }

    // beta part
    std::vector<double> beta(prob.nvars_last, 0.0);
    prob.transfer_block.multiply_transpose_with_vector(pi, beta);

    if (beta.size() == 0)
    {
        static bool warning_printed = false;
        if (!warning_printed)
        {
            // warn the user that the transfer block is empty
            std::cerr << "WARNING: CutHelper::get_static_part: transfer block is empty." << std::endl;
            warning_printed = true;
        }
    }
    return {alpha, beta};
}

void CutHelper::add_dynamic_part(const StageProblem &prob, const std::vector<double> &pi, const std::vector<double> &scenario, Cut &cut)
{
    const StageStochasticPattern &pattern = prob.stage_stoc_pattern;

    // make sure
    if (pattern.rv_count != scenario.size())
        throw std::runtime_error("CutHelper::add_dynamic_part: scenario size does not match the number of random variables.");

    for (size_t i = 0; i < pattern.rv_count; ++i)
    {
        // pattern.row_index[i], pattern.col_index[i] are the row and column indices of the nonzero element
        if (pattern.col_index[i] == -1)
        {
            // should be added to the intercept
            cut.alpha += scenario[i] * pi[pattern.row_index[i]];
        }
        else
        {
            // transfer block
            cut.beta[pattern.col_index[i]]  += scenario[i] * pi[pattern.row_index[i]];
        }
    }
}
