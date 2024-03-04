#ifndef SCS_H
#define SCS_H
#include <vector>

// Implementation of SCS algorithm
class SCS
{
public:
    // perform SCS update step using the given gradient
    void update(const std::vector<double> &grad);

    // get the current direction
    const std::vector<double> &get_current_direction() const;

protected:
    // current direction
    std::vector<double> current_direction;

    // norm of the current direction
    double d_norm_squared;

private:
    // whether the algorithm is initialized: first call to update will initialize the algorithm
    bool initialized = false;

    // calculate optimal lambda according to the SCS algorithm
    static double optimal_lambda(double dg, double gg, double dd);

};

class UnconstrainedSCS : public SCS
{
public:
    // satisfy L condition
    // f_forward: objective value at the forward point x_forward
    // f_current: objective value at the current point x
    // t: step size
    bool satisfy_L_condition(double f_forward, double f_current, double t) const;

    // satisfy R condition
    // g_forward: gradient at the forward point x_forward
    bool satisfy_R_condition(const std::vector<double> &g_forward) const;

private:
    // m1 parameter
    static constexpr double m1 = 0.4;

    // m2 parameter
    static constexpr double m2 = 0.2;
};

class ConstraintedSCS : public SCS
{
    public:
    // returns true if satisfy L condition
    // f_forward: objective value at the forward point x_forward
    // f_current: objective value at the current point x
    // x_forward: forward point
    // x: current point
    // t: step size
    bool satisfy_L_condition_constrained(double f_forward, double f_current, const std::vector<double> &x_forward, const std::vector<double> &x, double t) const;

    // returns true if satisfy R condition
    // g_forward: gradient at the forward point x_forward
    // x_forward: forward point
    // x: current point
    bool satisfy_R_condition_constrained(const std::vector<double> &g_forward, const std::vector<double> &x_forward, const std::vector<double> &x) const;
};
#endif // SCS_H