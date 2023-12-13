#define CATCH_CONFIG_MAIN
#include "../external/catch_amalgamated.hpp"
#include "smps.h"
#include <limits>
#include <tuple>

#include <iostream>

using Catch::Approx;

TEST_CASE("SMPSCore COR File Parsing", "[SMPSCore]") {
    smps::SMPSCore core("tests/lands/lands.cor");

    SECTION("Read COR file and verify contents") {
        // Test problem name
        REQUIRE(core.problem_name == "LandS");

        // Test the number of columns and rows
        REQUIRE(core.num_cols == 16); // 16 columns
        REQUIRE(core.num_rows == 10);  // 10 rows (including the objective)

        // Test column names
        REQUIRE(core.col_name_map.get_name(0) == "X1");
        REQUIRE(core.col_name_map.get_name(1) == "X2");

        // Test row names
        REQUIRE(core.row_name_map.get_name(0) == "OBJ");
        REQUIRE(core.row_name_map.get_name(1) == "S1C1");

        // Test RHS values
        // Note the first row is the OBJ row, so the first RHS value is for the second row
        REQUIRE(core.rhs_coefficients.size() == core.num_rows);
        REQUIRE(core.rhs_coefficients[1] == Approx(12.0));
        REQUIRE(core.rhs_coefficients[2] == Approx(120.0));

        // Test inequality directions
        REQUIRE(core.inequality_directions.size() == core.num_rows);
        REQUIRE(core.inequality_directions[0] == 'N');
        REQUIRE(core.inequality_directions[1] == 'G');

        // Test lower and upper bounds
        // Example: Test one of the lower bounds is 0.0, and an upper bound is +inf
        REQUIRE(core.lower_bounds[0] == 0.0); // Assuming 0.0 is the lower bound for the first variable
        REQUIRE(core.upper_bounds[0] == std::numeric_limits<double>::infinity()); // Assuming +inf for the first variable

        // Testing LP coefficients
        REQUIRE(core.lp_coefficients.get_element(0, 0) == Approx(10.0));
        
    }
}

TEST_CASE("SMPSCore Implicit TIME File Parsing", "[SMPSImplicitTime]") {
    smps::SMPSCore cor("tests/lands/lands.cor");
    smps::SMPSImplicitTime tim("tests/lands/lands.tim");

    
    SECTION("Row tests") {
        REQUIRE(tim.get_row_stage("OBJ", cor.row_name_map) == std::make_tuple(-1, -1));
        REQUIRE(tim.get_row_stage("S1C1", cor.row_name_map) == std::make_tuple(0, 0));
        REQUIRE(tim.get_row_stage("S1C2", cor.row_name_map) == std::make_tuple(0, 1));
        REQUIRE(tim.get_row_stage("S2C1", cor.row_name_map) == std::make_tuple(1, 0));
        REQUIRE(tim.get_row_stage("S2C7", cor.row_name_map) == std::make_tuple(1, 6));

    }

    SECTION("Column tests") {
        REQUIRE(tim.get_col_stage("RHS", cor.col_name_map) == std::make_tuple(-1, -1));
        REQUIRE(tim.get_col_stage("X1", cor.col_name_map) == std::make_tuple(0, 0));
        REQUIRE(tim.get_col_stage("X4", cor.col_name_map) == std::make_tuple(0, 3));
        REQUIRE(tim.get_col_stage("Y11", cor.col_name_map) == std::make_tuple(1, 0));
        REQUIRE(tim.get_col_stage("Y21", cor.col_name_map) == std::make_tuple(1, 1));
        REQUIRE(tim.get_col_stage("Y42", cor.col_name_map) == std::make_tuple(1, 7));
    }

    SECTION("Size Tests") {
        REQUIRE(tim.nrows(0, cor.row_name_map) == 2);
        REQUIRE(tim.nrows(1, cor.row_name_map) == 7);
        REQUIRE(tim.ncols(0, cor.col_name_map) == 4);
        REQUIRE(tim.ncols(1, cor.col_name_map) == 12);
    }

}