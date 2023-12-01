#ifndef PROB_H
#define PROB_H

#include "smps.h" // Include smps.h for SMPSCore and SMPSTime

namespace sp {

    class StageProblem {
    public:
        // Constructor and other necessary methods

        void* solverEnvironment;
        void* lpInstance;
        smps::SMPSCore* smpsCore;

        virtual void buildProblem(const smps::SMPSCore& core, const smps::SMPSTime& time, int stage) = 0;
        virtual void solveProblem(const std::vector<double>& stateVariables, double& primalSolution, double& dualSolution, double& optimalValue) = 0;
    };

    // More specific derived classes for different solvers can be defined here.
}

#endif // PROB_H
