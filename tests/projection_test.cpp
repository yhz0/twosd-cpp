#define CATCH_CONFIG_MAIN
#include "../external/catch_amalgamated.hpp"

#include "projection.h"
#include "smps.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("Test Projection Procedure", "[StageProjectionProblem]")
{
    // construct root stage problem
    smps::SMPSCore cor("tests/lands/lands.cor");
    smps::SMPSImplicitTime tim("tests/lands/lands.tim");
    smps::SMPSStoch sto("tests/lands/lands.sto");

    StageProjectionProblem prob(cor, tim, sto, 0);
    prob.attach_solver();

    // feasible vector: [0 12 0 0], should return nullopt
    auto out1 = prob.solve({0, 12, 0, 0});
    REQUIRE(!out1.has_value());

    // feasible and binding vector: [0 8 4 0]
    auto out2 = prob.solve({0, 8, 4, 0});
    REQUIRE(!out2.has_value());

    const double TOL = 0.001;
    // infeasible vector: [4 3 3 3] (violates the second constraint)
    // expect projection is 3.84127 2.88889 2.74603 2.90476
    // so the difference is -0.15873 -0.11111 -0.25397 -0.09524
    auto out3 = prob.solve({4, 3, 3, 3});
    REQUIRE(out3.has_value());
    REQUIRE(out3.value().size() == 4);
    REQUIRE_THAT(out3.value()[0], WithinAbs(-0.15873, TOL));
    REQUIRE_THAT(out3.value()[1], WithinAbs(-0.11111, TOL));
    REQUIRE_THAT(out3.value()[2], WithinAbs(-0.25397, TOL));
    REQUIRE_THAT(out3.value()[3], WithinAbs(-0.09524, TOL));

    // dump solver
    // GRBwrite(prob.get_model(), "projection_3.lp");

    // infeasible vector: [-1 13 0 0] (violates the bound constraint)
    // expect projection is 0, 13, 0, 0
    // difference is 1, 0, 0, 0
    auto out4 = prob.solve({-1, 13, 0, 0});
    REQUIRE(out4.has_value());
    REQUIRE(out4.value().size() == 4);
    REQUIRE_THAT(out4.value()[0], WithinAbs(1.0, TOL));
    REQUIRE_THAT(out4.value()[1], WithinAbs(0.0, TOL));
    REQUIRE_THAT(out4.value()[2], WithinAbs(0.0, TOL));
    REQUIRE_THAT(out4.value()[3], WithinAbs(0.0, TOL));

    // dump solver
    // GRBwrite(prob.get_model(), "projection_4.lp");
}