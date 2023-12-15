#define CATCH_CONFIG_MAIN
#include "../external/catch_amalgamated.hpp"

#include "smps.h"
#include "pattern.h"

using Catch::Approx;

TEST_CASE("Test StochasticPattern from_smps on instances", "[StochasticPattern]")
{
    SECTION("Test on LandS instance (indep discrete, rhs)")
    {
        smps::SMPSCore cor("tests/lands/lands.cor");
        smps::SMPSImplicitTime tim("tests/lands/lands.tim");
        smps::SMPSStoch sto("tests/lands/lands.sto");

        StochasticPattern pattern = StochasticPattern::from_smps(cor, tim, sto);

        // Expected values
        int expected_col_index = -1;
        int expected_row_index = 4;
        int expected_stage = 1;
        double expected_reference_value = 0.0;

        // Check that there is only one stochastic element
        REQUIRE(pattern.col_index.size() == 1);
        REQUIRE(pattern.row_index.size() == 1);
        REQUIRE(pattern.stage.size() == 1);
        REQUIRE(pattern.reference_values.size() == 1);

        // Check that the properties of the single stochastic element are as expected
        REQUIRE(pattern.col_index[0] == expected_col_index);
        REQUIRE(pattern.row_index[0] == expected_row_index);
        REQUIRE(pattern.stage[0] == expected_stage);
        REQUIRE(pattern.reference_values[0] == Approx(expected_reference_value));
    }

    SECTION("Test on transship instance (indep discrete, rhs)")
    {
        smps::SMPSCore cor("tests/transship/transship.cor");
        smps::SMPSImplicitTime tim("tests/transship/transship.tim");
        smps::SMPSStoch sto("tests/transship/transship.sto");

        StochasticPattern pattern = StochasticPattern::from_smps(cor, tim, sto);

        // Expected values for each random element
        std::vector<int> expected_col_indices(7, -1); // All values are -1
        std::vector<int> expected_row_indices = {28, 29, 30, 31, 32, 33, 34};
        int expected_stage = 1;
        double expected_reference_value = 0.0;

        // Check that there are exactly 7 stochastic elements
        REQUIRE(pattern.col_index.size() == 7);
        REQUIRE(pattern.row_index.size() == 7);
        REQUIRE(pattern.stage.size() == 7);
        REQUIRE(pattern.reference_values.size() == 7);

        // Check properties of each stochastic element
        for (int i = 0; i < 7; ++i)
        {
            REQUIRE(pattern.col_index[i] == expected_col_indices[i]);
            REQUIRE(pattern.row_index[i] == expected_row_indices[i]);
            REQUIRE(pattern.stage[i] == expected_stage);
            REQUIRE(pattern.reference_values[i] == Approx(expected_reference_value));
        }
    }
}
