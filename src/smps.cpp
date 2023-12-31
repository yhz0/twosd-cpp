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
                if (section == "ROWS")
                {
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
                }
                else if (section == "COLUMNS")
                {
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
                    if (!col_index.has_value())
                    {
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
                    else
                    {
                        current_col_index = col_index.value();
                    }

                    std::string row_name;
                    double coefficient;
                    while (iss >> row_name >> coefficient)
                    {
                        std::optional<int> row_index = row_name_map.get_index(row_name);
                        int current_row_index;
                        if (!row_index.has_value())
                        {
                            // the row name is not in the map
                            // throw an exception
                            throw std::runtime_error("Row name '" + row_name + "' not found at line " + std::to_string(line_number));
                        }
                        else
                        {
                            current_row_index = row_index.value();
                        }

                        // add the coefficient to the lp coefficient matrix
                        lp_coefficients.add_element(current_row_index, current_col_index, coefficient);
                    }
                }
                else if (section == "RHS")
                {
                    // rhs line has three tokens
                    // the first token is always RHS
                    // the second token is the name of the row
                    // the third token is the coefficient

                    std::string dummy, row_name;
                    double coefficient;
                    iss >> dummy;
                    
                    // make sure the dummy reads "RHS"
                    if (dummy != "RHS")
                    {
                        throw std::runtime_error("Expected 'RHS' at line " + std::to_string(line_number));
                    }

                    while (iss >> row_name >> coefficient)
                    {
                        std::optional<int> row_index = row_name_map.get_index(row_name);
                        if (!row_index.has_value())
                        {
                            // the row name is not in the map
                            // throw an exception
                            throw std::runtime_error("Row name '" + row_name + "' not found at line " + std::to_string(line_number));
                        }
                        else
                        {
                            int current_row_index = row_index.value();
                            rhs_coefficients[current_row_index] = coefficient;
                        }
                    }

                }
                else if (section == "BOUNDS")
                {
                    // bound lines has four tokens
                    // the first token is either UP (upper) or LO (lower) or FX (fixed) or FR (free)
                    // the second token is the bound name
                    // the third token is the name of the column
                    // the fourth token is the bound value

                    std::string bound_type, dummy, col_name;
                    double bound_value;

                    iss >> bound_type >> dummy >> col_name >> bound_value;

                    std::optional<int> col_index = col_name_map.get_index(col_name);
                    if (!col_index.has_value())
                    {
                        // the column name is not in the map
                        // throw an exception
                        throw std::runtime_error("Column name '" + col_name + "' not found at line " + std::to_string(line_number));
                    }
                    else
                    {
                        int current_col_index = col_index.value();
                        if (bound_type == "UP")
                        {
                            upper_bounds[current_col_index] = bound_value;
                        }
                        else if (bound_type == "LO")
                        {
                            lower_bounds[current_col_index] = bound_value;
                        }
                        else if (bound_type == "FX")
                        {
                            lower_bounds[current_col_index] = bound_value;
                            upper_bounds[current_col_index] = bound_value;
                        }
                        else if (bound_type == "FR")
                        {
                            lower_bounds[current_col_index] = -std::numeric_limits<double>::infinity();
                            upper_bounds[current_col_index] = std::numeric_limits<double>::infinity();
                        }
                        else
                        {
                            throw std::runtime_error("Unsupported bound type '" + bound_type + "' found at line " + std::to_string(line_number));
                        }
                    }
                }
                else
                {
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

    std::tuple<int, int> SMPSImplicitTime::get_row_stage(std::string row_name, const BijectiveMap &row_name_map) const
    {
        // identify the objective row, assuming it is the first row
        if (row_name == row_name_map.get_name(0).value())
        {
            return std::tuple<int, int>(-1, -1);
        }

        int stage = 0, cnt = 0;
        for (int i = 0; i < row_name_map.size(); i++)
        {
            std::string current_name = row_name_map.get_name(i).value();

            if ((unsigned)stage < row_names.size() && row_names[stage] == current_name)
            {
                stage++;
                cnt = 0;
            }

            // skip the count for the objective row
            if (current_name != row_name_map.get_name(0).value())
                cnt++;

            if (current_name == row_name)
            {
                return std::tuple<int, int>(stage - 1, cnt - 1);
            }
        }

        throw std::runtime_error("Invalid row_name in get_row_stage!");
    }

    std::tuple<int, int> SMPSImplicitTime::get_col_stage(std::string col_name, const BijectiveMap &col_name_map) const
    {
        if (col_name == "RHS" || col_name == "rhs")
        {
            return std::tuple<int, int>(-1, -1);
        }

        int stage = 0, cnt = 0;
        for (int i = 0; i < col_name_map.size(); i++)
        {
            std::string current_name = col_name_map.get_name(i).value();

            if ((unsigned)stage < column_names.size() && column_names[stage] == current_name)
            {
                stage++;
                cnt = 0;
            }
            cnt++;
            if (current_name == col_name)
            {
                return std::tuple<int, int>(stage - 1, cnt - 1);
            }
        }

        throw std::runtime_error("Invalid col in get_col_stage!");
    }

    int SMPSTime::nrows(int stage, const BijectiveMap &row_name_map) const
    {
        int cnt = 0;
        for (int i = 0; i < row_name_map.size(); i++)
            if (std::get<0>(get_row_stage(row_name_map.get_name(i).value(), row_name_map)) == stage)
                cnt++;
        return cnt;
    }

    int SMPSTime::ncols(int stage, const BijectiveMap &col_name_map) const
    {
        int cnt = 0;
        for (int i = 0; i < col_name_map.size(); i++)
            if (std::get<0>(get_col_stage(col_name_map.get_name(i).value(), col_name_map)) == stage)
                cnt++;
        return cnt;
    }

    double SMPSStoch::SMPSIndepDiscrete::generate(std::mt19937 &rng)
    {
        return values[dist(rng)];
    }

    std::string SMPSStoch::SMPSIndepDiscrete::element_summary() const
    {
        // enumerate all the values
        std::string s = "INDEP DISCRETE Values: ";
        for (auto v: values) {
            s += std::to_string(v) + " ";
        }
        s += " ";
        // enumerate all the probs
        s += "Probs: ";
        for (auto p: dist.probabilities()) {
            s += std::to_string(p) + " ";
        }

        return s;
    }

    double SMPSStoch::SMPSIndepNormal::generate(std::mt19937 &rng)
    {
        return dist(rng);
    }

    std::string SMPSStoch::SMPSIndepNormal::element_summary() const
    {
        //print mean and stddev
        return "INDEP NORMAL Mean: " + std::to_string(dist.mean()) + " Stddev: " + std::to_string(dist.stddev());
    }

    double SMPSStoch::SMPSIndepUniform::generate(std::mt19937 &rng)
    {
        return dist(rng);
    }

    std::string SMPSStoch::SMPSIndepUniform::element_summary() const
    {
        // print lower and upper
        return "INDEP UNIFORM Lower: " + std::to_string(dist.a()) + " Upper: " + std::to_string(dist.b());
    }

    SMPSStoch::SMPSStoch(const std::string &filename)
    {
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open())
        {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        int line_number = 0;

        // current section headers
        std::string section_name, subsection_name;

        // need special treatment for INDEP DISCRETE
        // this will be remembered and processed in the end
        // maps from (col, row) -> vector of (value, prob)
        std::map< std::tuple<std::string, std::string>,
            std::vector<std::tuple<double, double> > > indep_discrete_mapping;
        
        // the indices in indep_elem that correspond to the positions in indep_pos
        // we should process them in the end
        std::vector<int> indep_discrete_mapping_index;

        while (getline(file, line))
        {
            line_number++;
            std::istringstream iss(line);

            // Ignore empty lines and comments
            if (line.empty() || line[0] == '*')
            {
                continue;
            }

            if (line[0] == ' ')
            {
                // data lines.
                if (section_name == "INDEP" && subsection_name == "DISCRETE") {
                    // INDEP DISCRETE
                    // remember it just now and process at the end
                    std::string col_name, row_name;
                    double value, prob;
                    iss >> col_name >> row_name >> value >> prob;
                    
                    auto pos = std::make_tuple(col_name, row_name);
                    auto entry = std::make_tuple(value, prob);

                    // if indep_pos does not contain pos, then append it
                    if (std::find(indep_pos.begin(), indep_pos.end(), pos) == indep_pos.end()) {
                        indep_discrete_mapping_index.push_back(indep_pos.size());

                        indep_pos.push_back(pos);
                        // create a placeholder in indep_elem
                        indep_elem.push_back(nullptr);
                    }

                    // create empty vector for indep_discrete_mapping[pos] if not exists
                    // then append the current entry to it
                    if (indep_discrete_mapping.find(pos) == indep_discrete_mapping.end()) {
                        indep_discrete_mapping[pos] = std::vector<std::tuple<double, double> >();
                    }
                    indep_discrete_mapping[pos].push_back(entry);
                }
                else if (section_name == "INDEP" && subsection_name == "NORMAL") {
                    // INDEP NORMAL
                    std::string col_name, row_name;
                    double mean, stddev;
                    iss >> col_name >> row_name >> mean >> stddev;
                    
                    auto pos = std::make_tuple(col_name, row_name);
                    indep_pos.push_back(pos);
                    auto ptr = std::make_unique<SMPSIndepNormal>(mean, stddev);
                    indep_elem.push_back(std::move(ptr));
                }
                else if (section_name == "INDEP" && subsection_name == "UNIFORM") {
                    // INDEP UNIFORM
                    std::string col_name, row_name;
                    double lower, upper;
                    iss >> col_name >> row_name >> lower >> upper;
                    
                    auto pos = std::make_tuple(col_name, row_name);
                    indep_pos.push_back(pos);
                    auto ptr = std::make_unique<SMPSIndepUniform>(lower, upper);
                    indep_elem.push_back(std::move(ptr));
                }
                else {
                    // unsupported subsection
                    throw std::runtime_error("Unsupported subsection type '" + subsection_name + "' at line " + std::to_string(line_number));
                }
            }
            else
            {
                // this is a section switch
                iss >> section_name;

                // Read in problem name
                if (section_name == "STOCH")
                {
                    iss >> problem_name;
                }
                else if (section_name == "INDEP")
                {
                    iss >> subsection_name;

                    // Check if subsection_name is one of the supported types
                    if (subsection_name != "DISCRETE" && subsection_name != "NORMAL" && subsection_name != "UNIFORM")
                    {
                        throw std::runtime_error("Unsupported subsection type '" + subsection_name + "' at line " + std::to_string(line_number));
                    }
                }
                else if (section_name == "ENDATA")
                {
                    // Signifies end of sto file
                    break;
                }
                else
                {
                    // Handle other unexpected section names
                    throw std::runtime_error("Unsupported section '" + section_name + "' at line " + std::to_string(line_number));
                }
            }
        }

        // process indep_discrete_mapping
        for (size_t i = 0; i < indep_discrete_mapping_index.size(); i++) {
            int pos = indep_discrete_mapping_index[i];
            std::vector<double> values, probs;
            // find the entries corresponding to that col_name, row_name
            auto vps = indep_discrete_mapping[indep_pos[pos]];
            for (auto t: vps) {
                values.push_back(std::get<0>(t));
                probs.push_back(std::get<1>(t));
            }

            auto ptr = std::make_unique<SMPSIndepDiscrete>(values, probs);
            indep_elem[pos] = std::move(ptr);
        }
        file.close();
    }

    std::string SMPSStoch::summary() const
    {
        // print problem name
        std::string s = "PROBLEM NAME: " + problem_name + "\n";
        // loop through indep_pos and indep_elem to print out the summary
        s += "INDEP SECTION\n";
        for (size_t i = 0; i < indep_pos.size(); i++) {
            s += std::get<0>(indep_pos[i]) + " " + std::get<1>(indep_pos[i]) + " ";
            s += indep_elem[i]->element_summary() + "\n";
        }
        
        return s;
    }

    std::vector<double> SMPSStoch::generate_scenario(std::mt19937 &rng)
    {
        std::vector<double> omega;
        omega.resize(indep_elem.size());
        for(size_t i = 0; i < indep_elem.size(); i++) {
            omega[i] = indep_elem[i]->generate(rng);
        }
        return omega;
    }

    const std::vector<std::tuple<std::string, std::string>>& SMPSStoch::get_positions() const
    {
        return indep_pos;
    }

} // namespace smps
