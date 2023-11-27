#ifndef SMPS_CORE_H
#define SMPS_CORE_H

#include "sparse_matrix.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>

class SMPSCore {
public:
    SMPSCore();

    // Loads and parses the COR file
    bool load_core_file(const std::string& filename);

    // Display function (similar to Julia's show)
    friend std::ostream& operator<<(std::ostream& os, const SMPSCore& core);

private:
    std::string problem_name;
    std::vector<char> directions;
    std::vector<std::string> row_names;
    std::vector<std::string> col_names;
    SparseMatrix<double> template_matrix;
    std::vector<double> rhs;
    std::vector<double> lower_bound;
    std::vector<double> upper_bound;
    std::unordered_map<std::string, int> col_mapping;
    std::unordered_map<std::string, int> row_mapping;

};

#endif // SMPS_CORE_H
