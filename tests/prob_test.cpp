#define CATCH_CONFIG_MAIN
#include "../external/catch_amalgamated.hpp"
#include "smps.h"
#include "prob.h"
#include <random>

using Catch::Approx;

TEST_CASE("Test Setup for Problem", "[StageProblem]")
{
    smps::SMPSCore cor("tests/lands/lands.cor");
    smps::SMPSImplicitTime tim("tests/lands/lands.tim");
    smps::SMPSStoch sto("tests/lands/lands.sto");

    // root stage problem
    SECTION("from_smps test stage 0")
    {
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
        CHECK(prob.current_block.get_element(0, 1) == 1.0);  // S1C1 X2
        CHECK(prob.current_block.get_element(0, 2) == 1.0);  // S1C1 X3
        CHECK(prob.current_block.get_element(0, 3) == 1.0);  // S1C1 X4
        CHECK(prob.current_block.get_element(1, 0) == 10.0); // S1C2 X1
        CHECK(prob.current_block.get_element(1, 1) == 7.0);  // S1C2 X2
        CHECK(prob.current_block.get_element(1, 2) == 16.0); // S1C2 X3
        CHECK(prob.current_block.get_element(1, 3) == 6.0);  // S1C2 X4

        // Check lower bounds
        REQUIRE(prob.lb == std::vector<double>{0.0, 0.0, 0.0, 0.0});

        // Check upper bounds
        // Assuming that infinity is represented in some way, e.g., std::numeric_limits<double>::infinity()
        REQUIRE(prob.ub == std::vector<double>{INFINITY, INFINITY, INFINITY, INFINITY});

        // // Check x_base
        // REQUIRE(prob.x_base == std::vector<double>{0.0, 0.0, 0.0, 0.0});

        // Check rhs_bar
        REQUIRE(prob.rhs_bar == std::vector<double>{12.0, 120.0});

        // // Check rhs_shift
        // REQUIRE(prob.rhs_shift == std::vector<double>{0.0, 0.0});

        // Check cost
        REQUIRE(prob.cost_coefficients == std::vector<double>{10.0, 7.0, 16.0, 6.0});

        // // Check cost_shift
        // REQUIRE(prob.cost_shift == 0.0);

        // // Check solver
        // REQUIRE(prob.solver == nullptr);
    }

    // stage 1 problem
    SECTION("smps_test stage 1")
    {
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

        // // Check x_base
        // CHECK(prob.x_base == std::vector<double>(12, 0.0));

        // Check rhs_bar
        CHECK(prob.rhs_bar == std::vector<double>{0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 2.0});

        // // Check rhs_shift
        // CHECK(prob.rhs_shift == std::vector<double>(7, 0.0));

        // Check cost
        CHECK(prob.cost_coefficients == std::vector<double>{
                                            40.0, 45.0, 32.0, 55.0, 24.0, 27.0, 19.2, 33.0, 4.0, 4.5, 3.2, 5.5});

        // Check cost_shift
        CHECK(prob.get_cost_shift() == 0.0);

        // // Check solver
        // CHECK(prob.solver == nullptr);
    }

    // test changing scenario in stage 1
    SECTION("smps_test stage 1 scenario")
    {
        StageProblem prob = StageProblem::from_smps(cor, tim, sto, 1);

        prob.attach_solver();

        // construct a scenario and previous stage input
        std::vector<double> scenario_omega = {123.4},
                            prev_stage_input = {1.0, 2.0, 3.0, 4.0};

        // change the scenario
        prob.apply_scenario_rhs(prev_stage_input, scenario_omega);

        // expected: [1 2 3 4 123.4 3 2]
        std::vector<double> expected_rhs = {1.0, 2.0, 3.0, 4.0, 123.4, 3.0, 2.0};

        // call gurobi to get the rhs
        std::vector<double> current_rhs(expected_rhs.size());
        GRBgetdblattrarray(prob.get_model(), GRB_DBL_ATTR_RHS, 0, expected_rhs.size(), current_rhs.data());

        // compare
        CHECK(current_rhs == expected_rhs);
    }

    SECTION("set x_base")
    {
        StageProblem prob = StageProblem::from_smps(cor, tim, sto, 0);
        prob.attach_solver();

        // set x_base to [10 20 30 40] and apply rhs
        prob.set_x_base({10.0, 20.0, 30.0, 40.0});
        prob.apply_root_stage_rhs();

        // expected cost shift = dot ([10 7 16 6], [10 20 30 40]) = 960
        CHECK(prob.get_cost_shift() == 960.0);

        // rhs_bar: [12,120]
        // rhs_shift: [1 1 1 1; 10 7 16 6] * [10,20,30,40]
        // expect new rhs is rhs_bar - rhs_shift = [-88, -840]

        std::vector<double> expected_rhs = {-88.0, -840.0};
        std::vector<double> current_rhs(expected_rhs.size());
        GRBgetdblattrarray(prob.get_model(), GRB_DBL_ATTR_RHS, 0, expected_rhs.size(), current_rhs.data());
        CHECK(current_rhs == expected_rhs);

        // unset x_base and apply rhs
        prob.unset_x_base();
        prob.apply_root_stage_rhs();

        // expect cost shift = 0
        CHECK(prob.get_cost_shift() == 0.0);

        // expect rhs to be [12,120]
        expected_rhs = {12.0, 120.0};
        GRBgetdblattrarray(prob.get_model(), GRB_DBL_ATTR_RHS, 0, expected_rhs.size(), current_rhs.data());
        CHECK(current_rhs == expected_rhs);
    }
}

TEST_CASE("SMPS integrated test on lgsc instance", "[StageProblem]")
{
    smps::SMPSCore cor("tests/lgsc/lgsc.cor");
    smps::SMPSImplicitTime tim("tests/lgsc/lgsc.tim");
    smps::SMPSStoch sto("tests/lgsc/lgsc.sto");

    std::mt19937 rng(0);

    StageProblem prob0 = StageProblem::from_smps(cor, tim, sto, 0),
                 prob1 = StageProblem::from_smps(cor, tim, sto, 1);

    // nrow: 174 348
    // ncols: 602 1480
    REQUIRE(prob0.nrows == 174);
    REQUIRE(prob0.nvars_last == 0);
    REQUIRE(prob0.nvars_current == 602);

    REQUIRE(prob1.nrows == 348);
    REQUIRE(prob1.nvars_last == 602);
    REQUIRE(prob1.nvars_current == 1480);

    // prob0: current block (174, 602)
    REQUIRE(prob0.current_block.get_num_rows() == 174);
    REQUIRE(prob0.current_block.get_num_cols() == 602);

    // prob1: current block (348, 1480)
    REQUIRE(prob1.current_block.get_num_rows() == 348);
    REQUIRE(prob1.current_block.get_num_cols() == 1480);

    // prob1: transfer block (348, 602)
    REQUIRE(prob1.transfer_block.get_num_rows() == 348);
    REQUIRE(prob1.transfer_block.get_num_cols() == 602);

    StochasticPattern patt = StochasticPattern::from_smps(cor, tim, sto);

    // number of RV = 920 / 5 = 184
    CHECK(sto.get_indep_size() == 184);
    CHECK(patt.rv_count[0] == 0);
    CHECK(patt.rv_count[1] == 184);

    // all in stage 1
    bool all_in_stage_1 = true;
    for (size_t i = 0; i < patt.stage.size(); ++i)
    {
        if (patt.stage[i] != 1)
        {
            all_in_stage_1 = false;
            break;
        }
    }
    CHECK(all_in_stage_1);

    // check the 5th entry
    // RHS          mBC2R3Pr12
    // col index is -1
    // absolute row index is 254
    // so the relative row index is 254 - 174 = 80
    // reference value is 100 from COR file
    // Value 5 50 100 200 500
    // Probability 0.5 0.2 0.2 0.05 0.05

    CHECK(patt.col_index[4] == -1);
    CHECK(patt.row_index[4] == 254 - 174);
    CHECK(patt.reference_values[4] == Approx(100.0));

    // generate a scenario
    // its dimension should be the same as the number of RVs
    auto scenario = sto.generate_scenario(rng);
    CHECK(scenario.size() == 184);

    // should contain elements from 5, 50, 100, 200, 500
    bool scenario_ok = true;
    for (auto &x : scenario)
    {
        if (x != 5.0 && x != 50.0 && x != 100.0 && x != 200.0 && x != 500.0)
        {
            scenario_ok = false;
            break;
        }
    }
    CHECK(scenario_ok);
}
