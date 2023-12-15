#ifndef SMPS_H
#define SMPS_H

#include <vector>
#include <array>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iterator>
#include <random>
#include <memory>

#include "sparse.h" // for SparseMatrix
#include "utils.h"  // for BijectiveMap

namespace smps
{

    class SMPSCore
    {
    public:
        SMPSCore();

        // Read the COR file and store the data in the object
        SMPSCore(const std::string &filename);

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
        // Returns the stage number and the index relative to the
        // first row in the stage given a row name and a bijective mapping of row names.
        // The objective row does not count, and returns (-1, -1).
        // The root stage is counted as stage 0.
        virtual std::tuple<int, int> get_row_stage(std::string row_name, const BijectiveMap &row_name_map) const = 0;

        // Returns the stage number and the index relative to the
        // first column in the stage given a column name and a bijective mapping of column names.
        // If the given name is "RHS" or "rhs" then returns (-1, -1).
        virtual std::tuple<int, int> get_col_stage(std::string col_name, const BijectiveMap &col_name_map) const = 0;

        // Returns the numbers of rows in the given stage, excluding the objective row
        int nrows(int stage, const BijectiveMap &row_name_map) const;

        // Returns the numbers of columns in the given stage.
        int ncols(int stage, const BijectiveMap &col_name_map) const;
    };

    class SMPSImplicitTime : public SMPSTime
    {
    public:
        // Constructor: Reads an implicit SMPS time file and populates class members.
        // filename: Path to the implicit SMPS time file.
        SMPSImplicitTime(const std::string &filename);

        std::tuple<int, int> get_row_stage(std::string row_name, const BijectiveMap &row_name_map) const override;
        std::tuple<int, int> get_col_stage(std::string col_name, const BijectiveMap &col_name_map) const override;

    private:
        std::string problem_name;              // Name of the problem.
        std::vector<std::string> column_names; // Names of the first column in each stage.
        std::vector<std::string> row_names;    // Names of the first row in each stage.
        std::vector<std::string> period_names; // Names of the periods.
    };

    // Representation of SMPS sto input file.
    // Currently it only supports INDEP input.
    class SMPSStoch
    {
    public:
        SMPSStoch(const std::string &filename);

        // returns a string summary of what the smps contains for debugging purposes
        std::string summary() const;

        // generate a random scenario using the given rng.
        // the scenario is stored in omega, which is resized to the correct size
        std::vector<double> generate_scenario(std::mt19937 &rng);

        // returns (row_name, col_name) tuples describing the position of the random elements
        const std::vector<std::tuple<std::string, std::string>>& get_positions() const;

    private:
        // Nested abstract class for stochastic elements
        class SMPSIndepElement
        {
        public:
            // returns a random number of the specified distribution
            // using the given rng
            virtual double generate(std::mt19937 &rng) = 0;

            // returns a string summary of the element
            virtual std::string element_summary() const = 0;
        };

        // Concrete class for discrete stochastic elements
        class SMPSIndepDiscrete : public SMPSIndepElement
        {
        public:
            SMPSIndepDiscrete(const std::vector<double> &_values,
                              const std::vector<double> &_probs) : dist(_probs.begin(), _probs.end()), values(_values) {}
            double generate(std::mt19937 &rng) override;
            std::string element_summary() const override;

        private:
            std::discrete_distribution<int> dist;
            std::vector<double> values;
        };

        // Concrete class for normal stochastic elements
        class SMPSIndepNormal : public SMPSIndepElement
        {
        public:
            SMPSIndepNormal(double m, double s) : dist(m, s) {}
            double generate(std::mt19937 &rng) override;
            std::string element_summary() const override;

        private:
            std::normal_distribution<double> dist;
        };

        // Concrete class for uniform stochastic elements
        class SMPSIndepUniform : public SMPSIndepElement
        {
        public:
            SMPSIndepUniform(double lb, double ub) : dist(lb, ub) {}
            double generate(std::mt19937 &rng) override;
            std::string element_summary() const override;

        private:
            std::uniform_real_distribution<double> dist;
        };

        // instance name indicated by the sto file
        std::string problem_name;

        // position of the elements using (col_name, row_name) format
        std::vector<std::tuple<std::string, std::string>> indep_pos;

        // the distribution of random elements
        std::vector<std::unique_ptr<SMPSIndepElement>> indep_elem;
    };

}

#endif // SMPS_H
