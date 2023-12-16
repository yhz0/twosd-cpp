#define CATCH_CONFIG_MAIN
#include "../external/catch_amalgamated.hpp"
#include "smps.h"
#include "prob.h"

TEST_CASE("Test Setup for Problem", "[StageProblem]") {
    smps::SMPSCore cor("tests/lands/lands.cor");
    smps::SMPSImplicitTime tim("tests/lands/lands.tim");
    smps::SMPSStoch sto("tests/lands/lands.sto");

    // root stage problem
    SECTION("from_smps test stage 0") {
        StageProblem prob = StageProblem::from_smps(cor, tim, sto, 0);

        // root stage:
        // last stage vars: none
        // current stage vars: X1 X2 X3 X4
        // current stage rows names: S1C1 S1C2
        // transfer block (empty) (test use transfer_block.nnz() == 0)
        // direction: G, L
        // current block
        // (0, 0) => 1.0    // S1C1 X1
        // (0, 1) => 10.0   // S1C1 X2
        // (0, 2) => 1.0    // S1C1 X3
        // (0, 3) => 7.0    // S1C1 X4
        // (1, 0) => 1.0    // S1C2 X1
        // (1, 1) => 16.0   // S1C2 X2
        // (1, 2) => 1.0    // S1C2 X3
        // (1, 3) => 6.0    // S1C2 X4
        // lower bound
        // X1, X2, X3, X4  0.0 0.0 0.0 0.0
        // upper bound  all +infinity
        // x_base 0.0 0.0 0.0 0.0
        // rhs_bar 12.0 120.0
        // rhs_shift 0.0 0.0
        // inequality_directions G L
        // cost 10.0 7.0 16.0 6.0
        // cost_shift 0.0 0.0 0.0 0.0
        // do not test stage_stoc_pattern, it is tested elsewhere
        // solver nullptr

        // Check last stage vars
        REQUIRE(prob.nvars_last == 0);

        // Check current stage vars
        REQUIRE(prob.nvars_current == 4);
        REQUIRE(prob.current_stage_var_names == std::vector<std::string>{"X1", "X2", "X3", "X4"});

        // Check current stage rows names
        REQUIRE(prob.nrows == 2);
        REQUIRE(prob.current_stage_row_names == std::vector<std::string>{"S1C1", "S1C2"});

        // Check transfer block (empty)
        REQUIRE(prob.transfer_block.nnz() == 0);

        // Check directions
        REQUIRE(prob.inequality_directions == std::vector<char>{'G', 'L'});

        // Check current block
        REQUIRE(prob.current_block.nnz() == 8);
        CHECK(prob.current_block.get_element(0, 0) == 1.0);  // S1C1 X1
        CHECK(prob.current_block.get_element(0, 1) == 1.0); // S1C1 X2
        CHECK(prob.current_block.get_element(0, 2) == 1.0);  // S1C1 X3
        CHECK(prob.current_block.get_element(0, 3) == 1.0);  // S1C1 X4
        CHECK(prob.current_block.get_element(1, 0) == 10.0);  // S1C2 X1
        CHECK(prob.current_block.get_element(1, 1) == 7.0); // S1C2 X2
        CHECK(prob.current_block.get_element(1, 2) == 16.0);  // S1C2 X3
        CHECK(prob.current_block.get_element(1, 3) == 6.0);  // S1C2 X4

        // Check lower bounds
        REQUIRE(prob.lb == std::vector<double>{0.0, 0.0, 0.0, 0.0});

        // Check upper bounds
        // Assuming that infinity is represented in some way, e.g., std::numeric_limits<double>::infinity()
        REQUIRE(prob.ub == std::vector<double>{INFINITY, INFINITY, INFINITY, INFINITY});

        // Check x_base
        REQUIRE(prob.x_base == std::vector<double>{0.0, 0.0, 0.0, 0.0});

        // Check rhs_bar
        REQUIRE(prob.rhs_bar == std::vector<double>{12.0, 120.0});

        // Check rhs_shift
        REQUIRE(prob.rhs_shift == std::vector<double>{0.0, 0.0});

        // Check cost
        REQUIRE(prob.cost == std::vector<double>{10.0, 7.0, 16.0, 6.0});

        // Check cost_shift
        REQUIRE(prob.cost_shift == 0.0);

        // Check solver
        REQUIRE(prob.solver == nullptr);
    }

    // stage 1 problem
    SECTION("smps_test stage 1") {
        StageProblem prob = StageProblem::from_smps(cor, tim, sto, 1);

        // last stage vars: X1 X2 X3 X4
        // current stage vars: Y11 Y21 Y31 Y41 Y12 Y22 Y32 Y42 Y13 Y23 Y33 Y43
        // current stage rows names: S2C1 S2C2 S2C3 S2C4 S2C5 S2C6 S2C7
        // transfer_block (row, col) => value
        // (0, 0) => -1.0
        // (1, 1) => -1.0
        // (2, 2) => -1.0
        // (3, 3) => -1.0
        // direction L L L L G G G
        // current block (row, col, value)
        // (0,  0) => 1.0
        // (4,  0) => 1.0
        // (1,  1) => 1.0
        // (4,  1) => 1.0
        // (2,  2) => 1.0
        // (4,  2) => 1.0
        // (3,  3) => 1.0
        // (4,  3) => 1.0
        // (0,  4) => 1.0
        // (5,  4) => 1.0
        // (1,  5) => 1.0
        // (5,  5) => 1.0
        // (2,  6) => 1.0
        // (5,  6) => 1.0
        // (3,  7) => 1.0
        // (5,  7) => 1.0
        // (0,  8) => 1.0
        // (6,  8) => 1.0
        // (1,  9) => 1.0
        // (6,  9) => 1.0
        // (2, 10) => 1.0
        // (6, 10) => 1.0
        // (3, 11) => 1.0
        // (6, 11) => 1.0
        // lower bound all zero (12 zeros)
        // upper bound all +infinity
        // x_base all zero
        // rhs_bar 0.0 0.0 0.0 0.0 0.0 3.0 2.0
        // rhs_shift 0.0 0.0 0.0 0.0 0.0 0.0 0.0
        // cost 40.0 45.0 32.0 55.0 24.0 27.0 19.2 33.0 4.0 4.5 3.2 5.5
        // cost_shift 0.0
        // solver nullptr

        // Check last stage vars
        REQUIRE(prob.nvars_last == 4);
        CHECK(prob.last_stage_var_names == std::vector<std::string>{"X1", "X2", "X3", "X4"});

        // Check current stage vars
        REQUIRE(prob.nvars_current == 12);
        CHECK(prob.current_stage_var_names == std::vector<std::string>{
                                                  "Y11", "Y21", "Y31", "Y41", "Y12", "Y22", "Y32", "Y42", "Y13", "Y23", "Y33", "Y43"});

        // Check current stage rows names
        REQUIRE(prob.nrows == 7);
        CHECK(prob.current_stage_row_names == std::vector<std::string>{
                                                  "S2C1", "S2C2", "S2C3", "S2C4", "S2C5", "S2C6", "S2C7"});

        // Check transfer block
        CHECK(prob.transfer_block.nnz() == 4);
        CHECK(prob.transfer_block.get_element(0, 0) == -1.0);
        CHECK(prob.transfer_block.get_element(1, 1) == -1.0);
        CHECK(prob.transfer_block.get_element(2, 2) == -1.0);
        CHECK(prob.transfer_block.get_element(3, 3) == -1.0);

        // Check directions
        CHECK(prob.inequality_directions == std::vector<char>{'L', 'L', 'L', 'L', 'G', 'G', 'G'});

        // Check current block
        REQUIRE(prob.current_block.nnz() == 24); // Expecting 24 non-zero elements
        CHECK(prob.current_block.get_element(0, 0) == 1.0);
        CHECK(prob.current_block.get_element(4, 0) == 1.0);
        CHECK(prob.current_block.get_element(1, 1) == 1.0);
        CHECK(prob.current_block.get_element(4, 1) == 1.0);
        CHECK(prob.current_block.get_element(2, 2) == 1.0);
        CHECK(prob.current_block.get_element(4, 2) == 1.0);
        CHECK(prob.current_block.get_element(3, 3) == 1.0);
        CHECK(prob.current_block.get_element(4, 3) == 1.0);
        CHECK(prob.current_block.get_element(0, 4) == 1.0);
        CHECK(prob.current_block.get_element(5, 4) == 1.0);
        CHECK(prob.current_block.get_element(1, 5) == 1.0);
        CHECK(prob.current_block.get_element(5, 5) == 1.0);
        CHECK(prob.current_block.get_element(2, 6) == 1.0);
        CHECK(prob.current_block.get_element(5, 6) == 1.0);
        CHECK(prob.current_block.get_element(3, 7) == 1.0);
        CHECK(prob.current_block.get_element(5, 7) == 1.0);
        CHECK(prob.current_block.get_element(0, 8) == 1.0);
        CHECK(prob.current_block.get_element(6, 8) == 1.0);
        CHECK(prob.current_block.get_element(1, 9) == 1.0);
        CHECK(prob.current_block.get_element(6, 9) == 1.0);
        CHECK(prob.current_block.get_element(2, 10) == 1.0);
        CHECK(prob.current_block.get_element(6, 10) == 1.0);
        CHECK(prob.current_block.get_element(3, 11) == 1.0);
        CHECK(prob.current_block.get_element(6, 11) == 1.0);

        // Check lower bounds
        CHECK(prob.lb == std::vector<double>(12, 0.0));

        // Check upper bounds
        // Assuming that infinity is represented in some way, e.g., std::numeric_limits<double>::infinity()
        CHECK(prob.ub == std::vector<double>(12, INFINITY));

        // Check x_base
        CHECK(prob.x_base == std::vector<double>(12, 0.0));

        // Check rhs_bar
        CHECK(prob.rhs_bar == std::vector<double>{0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 2.0});

        // Check rhs_shift
        CHECK(prob.rhs_shift == std::vector<double>(7, 0.0));

        // Check cost
        CHECK(prob.cost == std::vector<double>{
                               40.0, 45.0, 32.0, 55.0, 24.0, 27.0, 19.2, 33.0, 4.0, 4.5, 3.2, 5.5});

        // Check cost_shift
        CHECK(prob.cost_shift == 0.0);

        // Check solver
        CHECK(prob.solver == nullptr);
    }
}