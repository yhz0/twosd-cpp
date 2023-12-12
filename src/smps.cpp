#include "smps.h"
#include <iostream>
#include <algorithm>

namespace smps
{

    SMPSCore::SMPSCore() : num_rows(0), num_cols(0) {}

    SMPSCore::SMPSCore(const std::string &filename) : num_rows(0), num_cols(0)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Unable to open COR file: " + filename);
        }

        const std::array<std::string, 6> supported_sections = {"NAME", "ROWS", "COLUMNS", "RHS", "BOUNDS", "ENDATA"};

        std::string line, section;
        int line_number = 0; // Track the line number for error reporting

        while (std::getline(file, line))
        {
            line_number++; // Increment line number with each read line

            // Remove empty lines and comments
            if (line.empty() || line[0] == '*')
                continue;

            std::istringstream iss(line);
            if (line[0] != ' ')
            {
                // Section line
                iss >> section;

                // the name section is special, it has two tokens
                // the second token is the name of the problem
                if (section == "NAME")
                {
                    iss >> problem_name;
                    // make sure the name is not empty
                    if (problem_name.empty())
                    {
                        throw std::runtime_error("Problem name cannot be empty");
                    }
                }
            }
            else
            {
                // Data line
                if (section == "ROWS") {
                    // row line has two tokens
                    // the first is either N or E or L or G
                    // the second is the name of the row

                    std::string direction, row_name;
                    iss >> direction >> row_name;
                    row_name_map.add(row_name, num_rows);
                    inequality_directions.push_back(direction[0]);
                    num_rows++;

                    // resize the lp coefficient matrix and rhs vector
                    lp_coefficients.resize(num_rows, num_cols);
                    rhs_coefficients.resize(num_rows);

                } else if (section == "COLUMNS") {
                    // columns row has 3 or 5 tokens.
                    // the first is the name of the column
                    // it is followed by one or two pairs of tokens
                    // the first token in the pair is the name of the row
                    // the second token in the pair is the coefficient

                    std::string col_name;
                    iss >> col_name;

                    int current_col_index;
                    // add the column name to the map if it is not already there
                    std::optional<int> col_index = col_name_map.get_index(col_name);
                    if (!col_index.has_value()) {
                        col_name_map.add(col_name, num_cols);
                        current_col_index = num_cols;
                        num_cols++;

                        // resize the lp coefficient matrix
                        lp_coefficients.resize(num_rows, num_cols);

                        // resize the bounds vectors
                        // assume the lower bound of the new element is default to 0
                        // and the upper bound of the new element is default to +infinity
                        lower_bounds.resize(num_cols, 0.0);
                        upper_bounds.resize(num_cols, std::numeric_limits<double>::infinity());
                    }
                    else {
                        current_col_index = col_index.value();
                    }
                    
                    std::string row_name;
                    double coefficient;
                    while(iss >> row_name >> coefficient)
                    {
                        std::optional<int> row_index = row_name_map.get_index(row_name);
                        int current_row_index;
                        if (!row_index.has_value()) {
                            // the row name is not in the map
                            // throw an exception
                            throw std::runtime_error("Row name '" + row_name + "' not found at line " + std::to_string(line_number));
                        }
                        else {
                            current_row_index = row_index.value();
                        }

                        // add the coefficient to the lp coefficient matrix
                        lp_coefficients.add_element(current_row_index, current_col_index, coefficient);
                    }
                    
                } else if (section == "RHS") {
                    // rhs line has three tokens
                    // the first token is always RHS
                    // the second token is the name of the row
                    // the third token is the coefficient

                    std::string dummy, row_name;
                    double coefficient;
                    iss >> dummy >> row_name >> coefficient;

                    // make sure the dummy reads "RHS"
                    if (dummy != "RHS") {
                        throw std::runtime_error("Expected 'RHS' at line " + std::to_string(line_number));
                    }

                    std::optional<int> row_index = row_name_map.get_index(row_name);
                    if (!row_index.has_value()) {
                        // the row name is not in the map
                        // throw an exception
                        throw std::runtime_error("Row name '" + row_name + "' not found at line " + std::to_string(line_number));
                    }
                    else {
                        int current_row_index = row_index.value();
                        rhs_coefficients[current_row_index] = coefficient;
                    }

                } else if (section == "BOUNDS") {
                    // bound lines has four tokens
                    // the first token is either UP (upper) or LO (lower) or FX (fixed) or FR (free)
                    // the second token is always BND
                    // the third token is the name of the column
                    // the fourth token is the bound value

                    std::string bound_type, dummy, col_name;
                    double bound_value;

                    iss >> bound_type >> dummy >> col_name >> bound_value;

                    // make sure the dummy reads "BND"
                    if (dummy != "BND") {
                        throw std::runtime_error("Expected 'BND' at line " + std::to_string(line_number));
                    }

                    std::optional<int> col_index = col_name_map.get_index(col_name);
                    if (!col_index.has_value()) {
                        // the column name is not in the map
                        // throw an exception
                        throw std::runtime_error("Column name '" + col_name + "' not found at line " + std::to_string(line_number));
                    }
                    else {
                        int current_col_index = col_index.value();
                        if (bound_type == "UP") {
                            upper_bounds[current_col_index] = bound_value;
                        } else if (bound_type == "LO") {
                            lower_bounds[current_col_index] = bound_value;
                        } else if (bound_type == "FX") {
                            lower_bounds[current_col_index] = bound_value;
                            upper_bounds[current_col_index] = bound_value;
                        } else if (bound_type == "FR") {
                            lower_bounds[current_col_index] = -std::numeric_limits<double>::infinity();
                            upper_bounds[current_col_index] = std::numeric_limits<double>::infinity();
                        } else {
                            throw std::runtime_error("Unsupported bound type '" + bound_type + "' found at line " + std::to_string(line_number));
                        }
                    }
                } else {
                    throw std::runtime_error("Unsupported section name '" + section + "' found at line " + std::to_string(line_number));
                }
            }
        }
        file.close();
    }

    SMPSImplicitTime::SMPSImplicitTime(const std::string &filename)
    {
        std::ifstream file(filename);
        std::string line;
        bool inPeriodsSection = false;

        if (!file.is_open())
        {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '*')
                continue; // Skip empty lines and comments

            std::istringstream iss(line);
            std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

            if (tokens.empty())
                continue;

            if (tokens[0] == "TIME")
            {
                if (tokens.size() != 2)
                    throw std::runtime_error("Invalid TIME format in file");
                problem_name = tokens[1];
            }
            else if (tokens[0] == "PERIODS")
            {
                inPeriodsSection = true;
            }
            else if (tokens[0] == "ENDATA")
            {
                break;
            }
            else if (inPeriodsSection && tokens.size() == 3)
            {
                column_names.push_back(tokens[0]);
                row_names.push_back(tokens[1]);
                period_names.push_back(tokens[2]);
            }
            else
            {
                throw std::runtime_error("Invalid implicit time file format");
            }
        }

        file.close();
    }

    int SMPSImplicitTime::get_row_stage(std::string row_name, const BijectiveMap &row_name_map)
    {
        if (row_name == "OBJ" || row_name == "obj")
        {
            return -1;
        }

        int stage = 0;
        for (int i = 0; i < row_name_map.size(); i++) {
            std::string current_name = row_name_map.get_name(i).value();

            if (stage < row_names.size() && row_names[stage] == current_name) {
                stage ++;
            }
            if (current_name == row_name) {
                return stage - 1;
            }
        }

        throw std::runtime_error("Invalid row_name in get_row_stage!");
    }

    int SMPSImplicitTime::get_col_stage(std::string col_name, const BijectiveMap &col_name_map)
    {
        if (col_name == "RHS" || col_name == "rhs")
        {
            return -1;
        }

        int stage = 0;
        for (int i = 0; i < col_name_map.size(); i++) {
            std::string current_name = col_name_map.get_name(i).value();

            if (stage < column_names.size() && column_names[stage] == current_name) {
                stage ++;
            }
            if (current_name == col_name) {
                return stage - 1;
            }
        }

        throw std::runtime_error("Invalid col in get_col_stage!");
    }
} // namespace smps
