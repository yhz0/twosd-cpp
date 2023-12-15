#include "sparse.h"
#include <stdexcept>

template class SparseMatrix<double>;
template class SparseMatrix<float>;

template <typename T>
SparseMatrix<T>::SparseMatrix() : num_rows(0), num_cols(0), sparsity_pattern(), values() {}

template <typename T>
SparseMatrix<T>::SparseMatrix(const SparsityPattern &_sparsity_pattern, const std::vector<T> &_values, size_t _num_rows, size_t _num_cols) : num_rows(_num_rows), num_cols(_num_cols), sparsity_pattern(_sparsity_pattern), values(_values)
{
    // make sure the size matches
    if (sparsity_pattern.size() != values.size())
    {
        throw std::invalid_argument("SparseMatrix: sparsity pattern and values size does not match");
    }
}

template <typename T>
void SparseMatrix<T>::resize(size_t _num_rows, size_t _num_cols)
{
    num_rows = _num_rows;
    num_cols = _num_cols;
}

template <typename T>
void SparseMatrix<T>::add_element(int row, int col, T value)
{
    sparsity_pattern.row_indices.push_back(row);
    sparsity_pattern.col_indices.push_back(col);
    values.push_back(value);
}

template <typename T>
size_t SparseMatrix<T>::nnz() const
{
    return values.size();
}

template <typename T>
T SparseMatrix<T>::get_element(int row, int col) const
{
    for (size_t i = 0; i < sparsity_pattern.size(); ++i)
    {
        if (sparsity_pattern.row_indices[i] == row && sparsity_pattern.col_indices[i] == col)
        {
            return values[i];
        }
    }

    return T();
}

template <typename T>
void SparseMatrix<T>::multiply_with_vector(const std::vector<T> &vec, std::vector<T> &result) const
{
    // make sure the dimension matches
    if (vec.size() != num_cols)
    {
        throw std::invalid_argument("SparseMatrix: vector dimension does not match");
    }

    // resize and initialize result vector
    result.resize(num_rows);
    std::fill(result.begin(), result.end(), T());

    // iterate through all non-zero elements and add to result
    for (size_t i = 0; i < sparsity_pattern.size(); ++i)
    {
        result[sparsity_pattern.row_indices[i]] += values[i] * vec[sparsity_pattern.col_indices[i]];
    }
}

template <typename T>
void SparseMatrix<T>::multiply_transpose_with_vector(const std::vector<T> &vec, std::vector<T> &result) const
{
    // make sure dimension matches
    if (vec.size() != num_rows)
    {
        throw std::invalid_argument("SparseMatrix: vector dimension does not match");
    }

    // resize and initialize result vector
    result.resize(num_cols);
    std::fill(result.begin(), result.end(), T());

    // iterate through all non-zero elements and add to result
    for (size_t i = 0; i < sparsity_pattern.size(); ++i)
    {
        result[sparsity_pattern.col_indices[i]] += values[i] * vec[sparsity_pattern.row_indices[i]];
    }
}

size_t SparsityPattern::size() const
{
    return row_indices.size();
}
