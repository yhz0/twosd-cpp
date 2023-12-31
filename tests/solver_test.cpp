#define CATCH_CONFIG_MAIN
#include "../external/catch_amalgamated.hpp"

#include "solver.h"

#include <fstream>
#include <string>
#include <regex>

TEST_CASE("Gurobi Solver Test", "[GurobiSolver]")
{
    smps::SMPSCore cor("tests/lands/lands.cor");
    smps::SMPSImplicitTime tim("tests/lands/lands.tim");
    smps::SMPSStoch sto("tests/lands/lands.sto");

    StageProblem prob0 = StageProblem::from_smps(cor, tim, sto, 0),
                 prob1 = StageProblem::from_smps(cor, tim, sto, 1);

    SECTION("Load prob0 LandS")
    {
        Solver solver0 = Solver::from_template(prob0);
        solver0.write_model("tests/solver_test_lands_0.lp");

        std::ifstream lpFile("tests/solver_test_lands_0.lp");
        REQUIRE(lpFile.is_open());

        std::stringstream buffer;
        buffer << lpFile.rdbuf();           // Read the whole file into a string stream
        std::string content = buffer.str(); // Convert to string

        std::regex objectiveRegex("10\\s+X1\\s*\\+\\s*7\\s+X2\\s*\\+\\s*16\\s+X3\\s*\\+\\s*6\\s+X4");
        std::regex s1c1Regex("S1C1:\\s*((1\\.0\\s*)|(1\\s*)|())X1\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())X2\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())X3\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())X4\\s*>=\\s*12");
        std::regex s1c2Regex("S1C2:\\s*10\\s+X1\\s*\\+\\s*7\\s+X2\\s*\\+\\s*16\\s+X3\\s*\\+\\s*6\\s+X4\\s*<=\\s*120");

        REQUIRE(std::regex_search(content, objectiveRegex));
        REQUIRE(std::regex_search(content, s1c1Regex));
        REQUIRE(std::regex_search(content, s1c2Regex));
    }

    SECTION("Load prob1 LandS")
    {
        Solver solver1 = Solver::from_template(prob1);
        solver1.write_model("tests/solver_test_1.lp");
        std::ifstream lpFile("tests/solver_test_1.lp");
        REQUIRE(lpFile.is_open());

        std::stringstream buffer;
        buffer << lpFile.rdbuf();           // Read the whole file into a string stream
        std::string content = buffer.str(); // Convert to string

        std::regex objectiveRegex("40\\s+Y11\\s*\\+\\s*45\\s+Y21\\s*\\+\\s*32\\s+Y31\\s*\\+\\s*55\\s+Y41\\s*\\+\\s*24\\s+Y12\\s*\\+\\s*27\\s+Y22\\s*\\+\\s*19\\.2\\s+Y32\\s*\\+\\s*33\\s+Y42\\s*\\+\\s*4\\s+Y13\\s*\\+\\s*4\\.5\\s+Y23\\s*\\+\\s*3\\.2\\s+Y33\\s*\\+\\s*5\\.5\\s+Y43");
        std::regex s2c1to4Regex("S2C[1-4]:\\s*((1\\.0\\s*)|(1\\s*)|())Y[1-4]1\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y[1-4]2\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y[1-4]3\\s*<=\\s*0");
        std::regex s2c5Regex("S2C5:\\s*((1\\.0\\s*)|(1\\s*)|())Y11\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y21\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y31\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y41\\s*>=\\s*0");
        std::regex s2c6Regex("S2C6:\\s*((1\\.0\\s*)|(1\\s*)|())Y12\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y22\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y32\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y42\\s*>=\\s*3");
        std::regex s2c7Regex("S2C7:\\s*((1\\.0\\s*)|(1\\s*)|())Y13\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y23\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y33\\s*\\+\\s*((1\\.0\\s*)|(1\\s*)|())Y43\\s*>=\\s*2");

        REQUIRE(std::regex_search(content, objectiveRegex));
        REQUIRE(std::regex_search(content, s2c1to4Regex));
        REQUIRE(std::regex_search(content, s2c5Regex));
        REQUIRE(std::regex_search(content, s2c6Regex));
        REQUIRE(std::regex_search(content, s2c7Regex));
    }
}