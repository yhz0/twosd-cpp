#ifndef SCS_H
#define SCS_H
#include <vector>

// Implementation of SCS algorithm
class SCS {
    public:

    // perform SCS update step using the given gradient
    void update(const std::vector<double>& grad);

    // get the norm of the current direction
    double get_norm_squared() const;

    // get the current direction
    const std::vector<double>& get_current_direction() const;

    // satisfy L condition
    bool satisfy_L_condition(double f_forward, double f_current, double t) const;

    // satisfy R condition
    bool satisfy_R_condition(const std::vector<double>& grad_forward) const;

    private:
    // whether the algorithm is initialized: first call to update will initialize the algorithm
    bool initialized = false;

    // norm of the current direction
    double d_norm_squared;

    // current direction
    std::vector<double> current_direction;

    // calculate optimal lambda according to the SCS algorithm
    static double optimal_lambda(double dg, double gg, double dd);

    // m1 parameter
    static constexpr double m1 = 0.4;

    // m2 parameter
    static constexpr double m2 = 0.2;
};
#endif // SCS_H