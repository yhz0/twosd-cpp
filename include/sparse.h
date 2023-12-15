#ifndef SPARSE_H
#define SPARSE_H

#include <vector>
#include <tuple>

/**
 * SparsityPattern is a class that denotes the position of non-zero vectors.
 * The positions are stored in the order they are added.
 * So when aligned with a vector, it can denote a sparse matrix.
 * It is assumed that no duplicate entries will be added.
 */
class SparsityPattern
{
public:
    // constructor
    SparsityPattern() : row_indices(), col_indices(){};
    SparsityPattern(const std::vector<int> &_rows, const std::vector<int> &_cols) : row_indices(_rows), col_indices(_cols){};

    // number of sparse elements
    size_t size() const;

    std::vector<int> row_indices; // row indices of non-zero elements
    std::vector<int> col_indices; // column indices of non-zero elements
};

// Template class for SparseMatrix.
// This class represents a sparse matrix where most elements are zero.
// It stores only non-zero elements to save space.
template <typename T>
class SparseMatrix
{
public:
    // Default constructor creates a matrix with no non-zero elements
    SparseMatrix();

    // Constructor that takes a sparsity pattern and a vector of values, row and column numbers.
    SparseMatrix(const SparsityPattern &_sparsity_pattern, const std::vector<T> &_values, size_t _num_rows, size_t _num_cols);

    // resize the matrix to prepare for matrix multiplication.
    void resize(size_t _num_rows, size_t _num_cols);

    // Adds a non-zero element to the sparse matrix at the specified position.
    // This function should not be called more than once for the same position,
    // as it will add multiple entries for that position rather than replacing the value.
    // This function has no size check.
    void add_element(int row, int col, T value);

    // Number of non-zero elements
    size_t nnz() const;

    // Retrieves the value of an element at a specified position in the matrix.
    // If no value has been explicitly added at the position, returns a default value (zero).
    // This function has no size check.
    T get_element(int row, int col) const;

    // Multiplies this sparse matrix with a dense vector (interpreted as a column vector),
    // storing the result in the result vector.
    // The result vector will be resized to appropriate size.
    // vec: The dense vector to be multiplied with the matrix.
    void multiply_with_vector(const std::vector<T> &vec, std::vector<T> &result) const;

    // Multiplies the transpose of the sparse matrix with a column vector,
    // storing the result in the result vector.
    // The result vector will be resized to appropriate size.
    // vec: The dense vector to be multiplied with the matrix.
    void multiply_transpose_with_vector(const std::vector<T> &vec, std::vector<T> &result) const;

    // Nested Iterator class to allow iterating through the elements.
    class Iterator
    {
    public:
        Iterator(const SparseMatrix &matrix, size_t pos = 0)
            : matrix_(matrix), pos_(pos) {}

        bool operator!=(const Iterator &other) const
        {
            return pos_ != other.pos_;
        }

        const Iterator &operator++()
        {
            pos_++;
            return *this;
        }

        std::pair<int, int> position() const
        {
            return std::make_pair(matrix_.sparsity_pattern.row_indices[pos_],
                                  matrix_.sparsity_pattern.col_indices[pos_]);
        }

        T value() const
        {
            return matrix_.values[pos_];
        }

        struct Element {
            std::tuple<int, int, T> data;

            Element(int row, int col, T val)
                : data(std::make_tuple(row, col, val)) {}
        };

        Element operator*() const {
            return Element(matrix_.sparsity_pattern.row_indices[pos_],
                           matrix_.sparsity_pattern.col_indices[pos_],
                           matrix_.values[pos_]);
        }

    private:
        const SparseMatrix &matrix_;
        size_t pos_;
    };

    // Begin iterator
    Iterator begin() const
    {
        return Iterator(*this);
    }

    // End iterator
    Iterator end() const
    {
        return Iterator(*this, values.size());
    }

private:
    size_t num_rows, num_cols;

    SparsityPattern sparsity_pattern;
    std::vector<T> values;
};

#endif // SPARSE_H
