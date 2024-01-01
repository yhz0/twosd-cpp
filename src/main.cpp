#include <iostream>

#include "smps.h"
#include "prob.h"
#include "solver.h"

int main() {
    // 1. load the problem
    // smps::SMPSCore cor("");
    // smps::SMPSImplicitTime tim("");
    // smps::SMPSStoch sto("");

    // 2. set up problem structures

    // 2.1 set up first stage and second stage.
    // sp::StageProblem masterprob(cor, tim, 0), subproblem(cor, tim, 1);
    
    // 2.2 set up first stage X's of appropriate size
    // std::vector<double> x_incumbent, x_candidate;

    // 2.3 set up dual vertices set (shared)
    // DualVertexSet dv;

    // 3. main loop
    // while not stopped
    {

        // 3.1 MASTER window: x_incumbent, x_candidate, x_timestamp, slack_indices[k]
        // 3.1(a) set offset variable to x_incumbent; this will update RHS for first stage constraints
        // 3.1(b) clear current epiconstraints and fetch cuts from all workers, add to current LP with the offset
        // 3.1(c) set prox term
        // 3.1(d) invoke solve LP. store x_candidate
        // 3.1(e) lock window; update x_incumbent, x_candidate, x_timestamp, slack cut indices.



        // 3.2 WORKER window: cuts[k], nsample[k]
        // 3.2(a) get timestamp.
        // if timestamp is newer than the local timestamp,
        {
            // this means that master has new info for this worker.
            // 3.2(b) get x_incumbent, x_candidate.
            // update the current x_timestamp
            // 3.2(c) remove old cuts indicated by the master on local copy
        }

        // omp parallel
        // 3.2 sample, update nsample[k]
        // 3.3 solve subproblems to get dual vertices; store omega, fixed part.
        // 3.4 add dual vertices to set. in case of gpu, sync dual vertices to gpu.
        // omp sync all threads
        // 3.5 argmax procedure to build two new cuts for each worker, record cut_nsample
        // 3.6 lock window and update the newest cuts[k]
    }

    return 0;
}

