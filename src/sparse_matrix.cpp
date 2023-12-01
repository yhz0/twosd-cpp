#include "sparse_matrix.h"
#include <stdexcept>

template <typename T>
SparseMatrix<T>::SparseMatrix() : num_rows(0), num_cols(0) {}

template <typename T>
SparseMatrix<T>::SparseMatrix(int rows, int cols)
    : num_rows(rows), num_cols(cols) {}

template <typename T>
void SparseMatrix<T>::resize(int rows, int cols)
{
    num_rows = rows;
    num_cols = cols;
}

template <typename T>
void SparseMatrix<T>::add_element(int row, int col, T value)
{
    // Check if the indices are within bounds
    if (row >= num_rows || col >= num_cols || row < 0 || col < 0)
    {
        throw std::out_of_range("Indices are out of bounds.");
    }
    elements.push_back(std::make_tuple(row, col, value));
}

template <typename T>
T SparseMatrix<T>::get_element(int row, int col) const
{
    // Return the value if found, else return 0 (or a default value)
    for (const auto &elem : elements)
    {
        if (std::get<0>(elem) == row && std::get<1>(elem) == col)
        {
            return std::get<2>(elem);
        }
    }
    return T(); // Returns default value for T (0 for numeric types)
}

template <typename T>
std::vector<T> SparseMatrix<T>::multiply_with_vector(const std::vector<T> &vec) const
{
    if (vec.size() != static_cast<typename std::vector<T>::size_type>(num_cols))
    {
        throw std::invalid_argument("Vector size does not match the number of columns in the matrix.");
    }

    std::vector<T> result(num_rows, T(0)); // Initialize the result vector with zeros

    for (const auto &element : elements)
    {
        int row, col;
        T value;
        std::tie(row, col, value) = element;

        result[row] += value * vec[col];
    }

    return result;
}

// Iterator methods
template<typename T>
typename SparseMatrix<T>::Iterator SparseMatrix<T>::begin() {
    return elements.begin();
}

template<typename T>
typename SparseMatrix<T>::ConstIterator SparseMatrix<T>::begin() const {
    return elements.begin();
}

template<typename T>
typename SparseMatrix<T>::Iterator SparseMatrix<T>::end() {
    return elements.end();
}

template<typename T>
typename SparseMatrix<T>::ConstIterator SparseMatrix<T>::end() const {
    return elements.end();
}

template class SparseMatrix<double>;
template class SparseMatrix<float>;
