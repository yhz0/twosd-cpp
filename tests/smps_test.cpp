#define CATCH_CONFIG_MAIN
#include "../external/catch_amalgamated.hpp"
#include "smps.h"
#include <limits>

using Catch::Approx;

TEST_CASE("SMPSCore COR File Parsing", "[SMPSCore]") {
    smps::SMPSCore core;

    SECTION("Read COR file and verify contents") {
        core.read_cor_file("tests/lands/lands.cor");

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
