#include "scs.h"
#include <cstddef>
// #define DEBUG_SCS
#ifdef DEBUG_SCS
    #include <iostream>
#endif
void SCS::update(const std::vector<double> &grad)
{
    // least norm
    if(!initialized) [[unlikely]]
    {
        current_direction = grad;
        initialized = true;
    } else [[likely]]
    {
        // dg = g^T d and gg = g^T g
        double dg = 0.0, gg = 0.0;
        for (size_t i = 0; i < current_direction.size(); i++)
        {
            dg += grad[i] * current_direction[i];
            gg += grad[i] * grad[i];
        }

        double lambda = optimal_lambda(dg, gg, d_norm_squared);
        for (size_t i = 0; i < current_direction.size(); i++)
        {
            current_direction[i] = lambda * current_direction[i] + (1.0 - lambda) * grad[i];
        }
    }

    // update norm
    double new_norm_squared = 0.0;
    for (size_t i = 0; i < current_direction.size(); i++)
    {
        new_norm_squared += current_direction[i] * current_direction[i];
    }

    d_norm_squared = new_norm_squared;
}

const std::vector<double> &SCS::get_current_direction() const
{
    return current_direction;
}

double SCS::optimal_lambda(double dg, double gg, double dd)
{
    double a = -dg + gg, b = dd - 2.0 * dg + gg;
    double lambda = a / b;

    // TODO: is this correct? if d =g then dd == gg == dg
    if (b == 0.0)
        return 1.0;

    if (lambda < 0.0)
    {
        lambda = 0.0;
    }
    else if (lambda > 1.0)
    {
        lambda = 1.0;
    }
    return lambda;
}

bool UnconstrainedSCS::satisfy_L_condition(double f_forward, double f_current, double t) const
{
    bool satisfied = f_forward <= f_current - m1 * t * d_norm_squared;

#ifdef DEBUG_SCS
    std::cout << "SCS L condition:"
    << "t = " << t
    << ", f_forward = " << f_forward
    << ", f_current = " << f_current
    << ", d_norm_squared = " << d_norm_squared
    << ", satisfied = " << satisfied << '\n';
#endif

    return satisfied;
}

bool UnconstrainedSCS::satisfy_R_condition(const std::vector<double> &g_forward) const
{
    // dot product of grad_forward and current_direction
    double dg = 0.0;
    for (size_t i = 0; i < current_direction.size(); i++)
    {
        dg += g_forward[i] * current_direction[i];
    }

    bool satisfied = dg >= - m2 * d_norm_squared;

#ifdef DEBUG_SCS
    std::cout << "SCS R condition:"
    << "dg = " << dg
    << ", d_norm_squared = " << d_norm_squared
    << ", satisfied = " << satisfied << '\n';
#endif

    return satisfied;
}

bool ConstraintedSCS::satisfy_L_condition_constrained(double f_forward, double f_current, const std::vector<double> &x_forward, const std::vector<double> &x, double t) const
{
    // FIXME: implement this
    return false;
}

bool ConstraintedSCS::satisfy_R_condition_constrained(const std::vector<double> &g_forward, const std::vector<double> &x_forward, const std::vector<double> &x) const
{
    // FIXME: implement this
    return false;
}
