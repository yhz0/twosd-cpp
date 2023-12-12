#ifndef SMPS_H
#define SMPS_H

#include <vector>
#include <array>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iterator>
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
        SMPSCore(const std::string& filename);

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

class SMPSTime
    {
    public:
        // Returns the stage number of a row given its name and a bijective mapping of row names.
        // The root stage is counted as stage 0.
        virtual int get_row_stage(std::string row_name, const BijectiveMap &row_name_map) = 0;

        // Returns the stage number of a column given its name and a bijective mapping of column names.
        virtual int get_col_stage(std::string col_name, const BijectiveMap &col_name_map) = 0;
    };

    class SMPSImplicitTime : public SMPSTime
    {
    public:
        // Constructor: Reads an implicit SMPS time file and populates class members.
        // filename: Path to the implicit SMPS time file.
        SMPSImplicitTime(const std::string &filename);

        int get_row_stage(std::string row_name, const BijectiveMap &row_name_map) override;
        int get_col_stage(std::string col_name, const BijectiveMap &col_name_map) override;

    private:
        std::string problem_name;             // Name of the problem.
        std::vector<std::string> column_names; // Names of the first column in each stage.
        std::vector<std::string> row_names;    // Names of the first row in each stage.
        std::vector<std::string> period_names; // Names of the periods.
    };

    class SMPSStoch
    {
        public:
            // read smps stoch file from file.
            SMPSStoch();
    };

}

#endif // SMPS_H
