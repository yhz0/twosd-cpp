#define CATCH_CONFIG_MAIN
#include "../external/catch_amalgamated.hpp"
#include "sparse.h"

using Catch::Approx;

TEST_CASE("SparseMatrix<float> functionality", "[SparseMatrix]")
{
    // Setup
    std::vector<int> row_indices = {0, 1, 2};
    std::vector<int> col_indices = {1, 0, 2};
    std::vector<float> values = {1.0f, 2.0f, 3.0f};
    SparseMatrix<float> matrix(row_indices, col_indices, values, 3, 3);

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

        for (auto iter = matrix.begin(); iter != matrix.end(); ++iter)
        {
            auto element = *iter;
            REQUIRE(element.row == std::get<0>(expectedElements[index]));
            REQUIRE(element.col == std::get<1>(expectedElements[index]));
            REQUIRE(element.val == Approx(std::get<2>(expectedElements[index])));
            ++index;
        }

        REQUIRE(index == matrix.nnz()); // Ensure all non-zero elements are iterated
    }

    SECTION("Test in-place subtraction") {
        std::vector<float> vec = {2.0f, 3.0f, 4.0f}; // Example vector for multiplication
        std::vector<float> result = {10.0f, 20.0f, 30.0f}; // Initialize result with non-zero values
        std::vector<float> expected_result = {10.0f - 1.0f * 3.0f, 20.0f - 2.0f * 2.0f, 30.0f - 3.0f * 4.0f}; // Expected result after subtraction

        matrix.subtract_multiply_with_vector(vec, result);

        REQUIRE(result.size() == expected_result.size());
        for (size_t i = 0; i < result.size(); ++i) {
            REQUIRE(result[i] == Approx(expected_result[i])); // Check each element
        }
    }
}

TEST_CASE("SparseMatrix<double> and CSR format functionality", "[SparseMatrix][CSR]")
{
    // Setup for SparseMatrix<double>
    std::vector<int> row_indices = {0, 1, 2};
    std::vector<int> col_indices = {1, 0, 2};
    std::vector<double> values = {1.0, 2.0, 3.0};
    SparseMatrix<double> matrix(row_indices, col_indices, values, 3, 3);

    // Convert to CSR format
    SparseMatrixCSR csrMatrix(matrix);

    SECTION("CSR format conversion")
    {
        // Testing CSR format
        REQUIRE(csrMatrix.getRowBegin().size() == 4);
        REQUIRE(csrMatrix.getRowBegin()[0] == 0);
        REQUIRE(csrMatrix.getRowBegin()[1] == 1);
        REQUIRE(csrMatrix.getRowBegin()[2] == 2);
        REQUIRE(csrMatrix.getRowBegin()[3] == 3);

        REQUIRE(csrMatrix.getColumnIndex().size() == 3);
        REQUIRE(csrMatrix.getColumnIndex()[0] == 1);
        REQUIRE(csrMatrix.getColumnIndex()[1] == 0);
        REQUIRE(csrMatrix.getColumnIndex()[2] == 2);

        REQUIRE(csrMatrix.getValues().size() == 3);
        REQUIRE(csrMatrix.getValues()[0] == Approx(1.0));
        REQUIRE(csrMatrix.getValues()[1] == Approx(2.0));
        REQUIRE(csrMatrix.getValues()[2] == Approx(3.0));
    }
}