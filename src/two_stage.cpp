#include "two_stage.h"
#include "scs.h"
#include "utils.h"
#include <filesystem>
#include "gurobi_c.h"
#include <vector>
#include <random>

namespace fs = std::filesystem;

TwoStageSP::TwoStageSP(const std::string base_path, const std::string prob_name, int nworkers_):
nworkers(nworkers_),
cor((fs::path(base_path) / prob_name / (prob_name + ".cor")).string()),
tim((fs::path(base_path) / prob_name / (prob_name + ".tim")).string()),
sto((fs::path(base_path) / prob_name / (prob_name + ".sto")).string())
{
    // construct stage problems
    prob0 = std::make_unique<StageProblem>(cor, tim, sto, 0);
    prob0->attach_solver();

    // create a second stage problem, and copy it nworkers times
    StageProblem prob1_tmp(cor, tim, sto, 1);

    for (int i = 0; i < nworkers; ++i) {
        prob1.push_back(std::make_unique<StageProblem>(prob1_tmp));
        prob1[i]->attach_solver();
    }

}

const int INITIAL_SAMPLES = 2000;
TwoStageSCS::TwoStageSCS(const std::string base_path, const std::string prob_name, int nworkers_)
    : TwoStageSP(base_path, prob_name, nworkers_)
{
    // create a projection problem
    proj_prob0 = std::make_unique<StageProjectionProblem>(cor, tim, sto, 0);
    proj_prob0->attach_solver();   
}

void TwoStageSCS::solve()
{
    // set prob1 to single thread mode
    for (int i = 0; i < nworkers; ++i) {
        int error;
        error = GRBsetintparam(prob1[i]->get_env(), GRB_INT_PAR_THREADS, 1);
        check_solver_error(error, "Error setting threads parameter");
    }
    
    // sample pool
    std::vector<std::vector<double>> samples;

    // random number generator
    std::mt19937 rng(0);

    // generate initial samples
    for(int i = 0; i < INITIAL_SAMPLES; ++i) {
        samples.push_back(sto.generate_scenario(rng));
    }

    std::vector<double> x(prob0 -> nvars_current, 0.0);
    SCS scs;

    const int MAX_ITER = 10;
    for (int iter = 0; iter <MAX_ITER; ++iter) 
    {
        // obtain some feasible first stage solution
        // by projecting the zero vector onto the feasible set
        auto proj_solution = proj_prob0->project(x);
        if (proj_solution.has_value())
        {
            for (size_t j = 0; j < x.size(); ++j)
                x[j] += proj_solution.value()[j];
        }

        // print feasible solution
        std::cout << "Feasible solution: " << vec_to_string(x) << '\n';

        double obj_value = 0.0;
        std::vector<double> grad(prob0->nvars_current, 0.0);

        // solve all subproblems
        auto sol_pair = solve_subproblems(x, samples);
        auto &solutions = sol_pair.first;
        auto &cuts = sol_pair.second;

        // accumulate gradient  = -beta
        for (size_t i = 0; i < samples.size(); ++i)
            for (size_t j = 0; j < prob0->nvars_current; ++j)
                grad[j] -= cuts[i].beta[j];

        // divide gradient by the number of samples
        for (size_t j = 0; j < prob0->nvars_current; ++j)
            grad[j] /= samples.size();

        // accumulate objective solution.obj_value
        for (size_t i = 0; i < samples.size(); ++i)
            obj_value += solutions[i].obj_value;
        
        // divide objective by the number of samples
        obj_value /= samples.size();

        // add the first stage objective
        for (size_t j = 0; j < x.size(); ++j)
            obj_value += prob0->cost_coefficients[j] * x[j];
    
        // print objective
        std::cout << "Objective value: " << obj_value << '\n';

        // print gradient
        // std::cout << "Gradient: " << vec_to_string(grad) << '\n';

        // gradient descent
        // x = x - grad * step_size
        // const double step_size = 0.01;
        // for (size_t j = 0; j < x.size(); ++j)
        //     x[j] -= grad[j] * step_size;

        scs.update(grad);
        std::vector<double> direction = scs.get_current_direction();

        // print direction
        std::cout << "SCS Direction: " << vec_to_string(direction) << '\t' << "Norm2: " << scs.get_norm_squared() << '\n';
        
        // // constant step size
        // const double step_size = 0.2;

        // line search
        double step_size = 0.0;



        std::cout << "Line search Step size: " << step_size << '\n';

        // descent step
        for (size_t j = 0; j < x.size(); ++j)
            x[j] -= direction[j] * step_size;
    }
}

std::pair<
    std::vector<StageProblem::Solution>,
    std::vector<Cut> > TwoStageSCS::solve_subproblems(const std::vector<double> &x, const std::vector<std::vector<double>> &samples)
{

    std::vector<StageProblem::Solution> solutions(samples.size());
    std::vector<Cut> cuts(samples.size());

    #pragma omp parallel for
    for (size_t i = 0; i < samples.size(); ++i)
    {
        int thread_id = omp_get_thread_num();
        StageProblem &prob1_inst = *prob1[thread_id];

        prob1_inst.update_solver_with_scenario(x, samples[i]);
        // solve the problem, require_dual_solution = true
        solutions[i] = prob1_inst.solve_problem(true);

        // get the static part of the cut
        cuts[i] = CutHelper::get_static_part(prob1_inst, solutions[i].dual_solution);

        // add the dynamic part of the cut
        CutHelper::add_dynamic_part(prob1_inst, solutions[i].dual_solution, samples[i], cuts[i]);

    }
    
    return std::make_pair(solutions, cuts);
}
