#ifndef SMPS_H
#define SMPS_H

#include <vector>
#include <array>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <cassert>

#include "sparse_matrix.h" // for SparseMatrix
#include "utils.h"         // for BijectiveMap

namespace smps
{

    class SMPSCore
    {
    public:
        SMPSCore();

        // Read the COR file and store the data in the object
        void read_cor_file(const std::string& filename);

        // the name of the problem
        std::string problem_name;

        // number of rows (constraints) and columns (variables)
        size_t num_rows;
        size_t num_cols;

        // Maps to convert between row/column names and indices
        BijectiveMap row_name_map;
        BijectiveMap col_name_map;

        // Data structures to store the LP problem
        SparseMatrix<double> lp_coefficients;
        std::vector<double> rhs_coefficients;
        std::vector<char> inequality_directions;
        std::vector<double> lower_bounds;
        std::vector<double> upper_bounds;

    };

    // class SMPSTime
    // {
    // public:
    //     virtual int get_stage(int index, const std::string &name_mapping) = 0;
    // };

    // class SMPSImplicitTime : public SMPSTime
    // {
    // public:
    //     int get_stage(int index, const std::string &name_mapping) override;

    // private:
    //     std::vector<std::string> column_names;
    //     std::vector<std::string> row_names;
    // };

    // class SMPSExplicitTime : public SMPSTime
    // {
    // public:
    //     int get_stage(int index, const std::string &name_mapping) override;
    
    // private:
    //     std::map<std::string, int> name_to_stage_map;
    // };

}

#endif // SMPS_H
