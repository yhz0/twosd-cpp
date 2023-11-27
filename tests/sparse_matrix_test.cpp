#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() function
#include "../external/catch_amalgamated.hpp"
#include "../include/sparse_matrix.h"

using Catch::Approx;

TEST_CASE("SparseMatrix is constructed correctly", "[SparseMatrix]") {
    SparseMatrix<double> matrix(3, 3);
    REQUIRE(matrix.get_element(0, 0) == 0.0);
}

TEST_CASE("SparseMatrix adds elements correctly", "[SparseMatrix]") {
    SparseMatrix<float> matrix(3, 3);
    matrix.add_element(1, 1, 2.5f);
    REQUIRE(matrix.get_element(1, 1) == Approx(2.5f));
}

TEST_CASE("SparseMatrix handles vector multiplication", "[SparseMatrix]") {
    SparseMatrix<double> matrix(2, 2);
    matrix.add_element(0, 0, 1.0);
    matrix.add_element(1, 1, 3.0);

    std::vector<double> vec = {2.0, 4.0};
    auto result = matrix.multiply_with_vector(vec);

    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == Approx(2.0));
    REQUIRE(result[1] == Approx(12.0));
}
