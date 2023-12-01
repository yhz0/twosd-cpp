#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

#include <vector>
#include <tuple>

// Template class for SparseMatrix.
// This class represents a sparse matrix where most elements are zero.
// It stores only non-zero elements to save space.
template<typename T>
class SparseMatrix {
public:
    // Iterator methods.
    using Element = std::tuple<int, int, T>;
    using Iterator = typename std::vector<Element>::iterator;
    using ConstIterator = typename std::vector<Element>::const_iterator;

    // Methods to provide iterators for the non-zero elements
    Iterator begin();
    ConstIterator begin() const;
    Iterator end();
    ConstIterator end() const;

    // Default constructor creates a matrix of size 0x0.
    SparseMatrix();

    // Constructor to create a sparse matrix of given dimensions.
    // rows: Number of rows in the matrix.
    // cols: Number of columns in the matrix.
    SparseMatrix(int rows, int cols);

    // Resizes the matrix to new dimensions.
    // Any existing data in the matrix is retained, but may become inaccessible
    // if the new size is smaller than the original size.
    // rows: New number of rows in the matrix.
    // cols: New number of columns in the matrix.
    void resize(int rows, int cols);

    // Adds a non-zero element to the sparse matrix at the specified position.
    // This function should not be called more than once for the same position,
    // as it will add multiple entries for that position rather than replacing the value.
    // row: Row index for the element to be added.
    // col: Column index for the element to be added.
    // value: The non-zero value to be added at the specified position.
    void add_element(int row, int col, T value);

    // Retrieves the value of an element at a specified position in the matrix.
    // If no value has been explicitly added at the position, returns a default value (zero).
    // row: Row index of the element to retrieve.
    // col: Column index of the element to retrieve.
    // Returns: Value at the specified position or a default value if not set.
    T get_element(int row, int col) const;

    // Multiplies this sparse matrix with a dense vector (interpreted as a column vector).
    // Performs the multiplication only for the non-zero elements stored in the matrix.
    // vec: The dense vector to be multiplied with the matrix.
    // Returns: A new vector representing the result of the multiplication.
    std::vector<T> multiply_with_vector(const std::vector<T>& vec) const;

private:
    int num_rows; // Number of rows in the matrix.
    int num_cols; // Number of columns in the matrix.
    std::vector<std::tuple<int, int, T>> elements; // Vector to store non-zero elements as tuples (row, col, value).
};

#endif // SPARSE_MATRIX_H
