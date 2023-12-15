#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() function
#include "../external/catch_amalgamated.hpp"
#include "sparse.h"

using Catch::Approx;

TEST_CASE("SparseMatrix<float> functionality", "[SparseMatrix]")
{
    // Setup
    SparsityPattern pattern({0, 1, 2}, {1, 0, 2});
    std::vector<float> values = {1.0f, 2.0f, 3.0f};
    SparseMatrix<float> matrix(pattern, values, 3, 3);

    SECTION("Setup and Multiplication")
    {
        // Test the setup
        REQUIRE(matrix.get_element(0, 1) == Approx(1.0f));
        REQUIRE(matrix.get_element(1, 0) == Approx(2.0f));
        REQUIRE(matrix.get_element(2, 2) == Approx(3.0f));
        REQUIRE(matrix.get_element(0, 0) == Approx(0.0f));

        REQUIRE(matrix.nnz() == 3);

        // Test multiplication
        std::vector<float> vec = {1.0f, 2.0f, 3.0f};
        std::vector<float> result;
        matrix.multiply_with_vector(vec, result);

        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == Approx(2.0f)); // 1.0 * 2.0
        REQUIRE(result[1] == Approx(2.0f)); // 2.0 * 1.0
        REQUIRE(result[2] == Approx(9.0f)); // 3.0 * 3.0
    }

    SECTION("Transpose Matrix-Vector Multiplication")
    {

        // Test multiplication with the transpose
        std::vector<float> vec = {1.0f, 2.0f, 3.0f};
        std::vector<float> result;
        matrix.multiply_transpose_with_vector(vec, result);

        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == Approx(4.0f)); // 2.0 * 2.0
        REQUIRE(result[1] == Approx(1.0f)); // 1.0 * 1.0
        REQUIRE(result[2] == Approx(9.0f)); // 3.0 * 3.0
    }

    SECTION("Iterator tests")
    {
        std::vector<std::tuple<int, int, float>> expectedElements = {
            {0, 1, 1.0f}, {1, 0, 2.0f}, {2, 2, 3.0f}};
        size_t index = 0;

        for (const auto &element : matrix)
        {
            REQUIRE(std::get<0>(element.data) == std::get<0>(expectedElements[index]));
            REQUIRE(std::get<1>(element.data) == std::get<1>(expectedElements[index]));
            REQUIRE(std::get<2>(element.data) == Approx(std::get<2>(expectedElements[index])));
            ++index;
        }

        REQUIRE(index == matrix.nnz()); // Ensure all non-zero elements are iterated
    }
}