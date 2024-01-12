#ifndef PROJECTION_H
#define PROJECTION_H

#include "prob.h"

class StageProjectionProblem: StageProblem {
    public:
    using StageProblem::StageProblem;

    // copy constructor
    StageProjectionProblem(const StageProjectionProblem& other);
    StageProjectionProblem& operator=(const StageProjectionProblem& other);

    // Declare the move constructor and move assignment operator
    StageProjectionProblem(StageProjectionProblem&& other) noexcept;
    StageProjectionProblem& operator=(StageProjectionProblem&& other) noexcept;

    void attach_solver() override;

};

#endif