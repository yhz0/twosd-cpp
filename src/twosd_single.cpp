#include "twosd_single.h"
#include <filesystem>

namespace fs = std::filesystem;

TwoSDSingle::TwoSDSingle(const std::string base_path, const std::string prob_name, int nworkers_):
nworkers(nworkers_)
{
    // construct path
    fs::path base_dir(base_path),
    cor_path = base_dir / prob_name / (prob_name + ".cor"),
    tim_path = base_dir / prob_name / (prob_name + ".tim"),
    sto_path = base_dir / prob_name / (prob_name + ".sto");

    smps::SMPSCore cor(cor_path.string());
    smps::SMPSImplicitTime tim(tim_path.string());
    smps::SMPSStoch sto(sto_path.string());

    // construct stage problems
    prob0 = std::make_unique<StageProblem>(StageProblem::from_smps(cor, tim, sto, 0));
    prob0->attach_solver();

    // create a second stage problem, and copy it nworkers times
    StageProblem prob1_tmp = StageProblem::from_smps(cor, tim, sto, 1);

    for (int i = 0; i < nworkers; ++i) {
        prob1.push_back(std::make_unique<StageProblem>(prob1_tmp));
        prob1[i]->attach_solver();
    }

}
