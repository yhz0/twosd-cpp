#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() function
#include "../external/catch_amalgamated.hpp"
#include "sparse_matrix.h"

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


TEST_CASE("SparseMatrix Iterators", "[SparseMatrix]") {
    SparseMatrix<float> matrix(3, 3); // Create a 3x3 sparse matrix
    matrix.add_element(0, 0, 1.0f);    // Add some non-zero elements
    matrix.add_element(1, 1, 2.0f);
    matrix.add_element(2, 2, 3.0f);

    SECTION("Iterator test") {
        std::vector<std::tuple<int, int, float>> expected = {
            std::make_tuple(0, 0, 1.0f),
            std::make_tuple(1, 1, 2.0f),
            std::make_tuple(2, 2, 3.0f)
        };
        auto it = expected.begin();
        for (auto& elem : matrix) {
            REQUIRE(*it == elem);
            ++it;
        }
    }

    SECTION("ConstIterator test") {
        const SparseMatrix<float>& constMatrix = matrix;
        std::vector<std::tuple<int, int, int>> expected = {
            std::make_tuple(0, 0, 1.0f),
            std::make_tuple(1, 1, 2.0f),
            std::make_tuple(2, 2, 3.0f)
        };
        auto it = expected.begin();
        for (const auto& elem : constMatrix) {
            REQUIRE(*it == elem);
            ++it;
        }
    }
}