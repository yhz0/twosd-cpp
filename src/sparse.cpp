#include "sparse.h"
#include <stdexcept>

template <typename T>
SparseMatrix<T>::SparseMatrix() : num_rows(0), num_cols(0) {}

template <typename T>
SparseMatrix<T>::SparseMatrix(const std::vector<int> &_row_indices, const std::vector<int> &_col_indices, 
                              const std::vector<T> &_values, size_t _num_rows, size_t _num_cols)
    : num_rows(_num_rows), num_cols(_num_cols), 
      row_indices(_row_indices), col_indices(_col_indices), values(_values) {}

template <typename T>
void SparseMatrix<T>::resize(size_t _num_rows, size_t _num_cols) {
    num_rows = _num_rows;
    num_cols = _num_cols;
}

template <typename T>
void SparseMatrix<T>::add_element(int row, int col, T value) {
    row_indices.push_back(row);
    col_indices.push_back(col);
    values.push_back(value);
}

template <typename T>
size_t SparseMatrix<T>::nnz() const {
    return values.size();
}

template <typename T>
T SparseMatrix<T>::get_element(int row, int col) const {
    for (size_t i = 0; i < values.size(); ++i) {
        if (row_indices[i] == row && col_indices[i] == col) {
            return values[i];
        }
    }
    return T(0);
}

template <typename T>
void SparseMatrix<T>::multiply_with_vector(const std::vector<T> &vec, std::vector<T> &result) const {
    result.assign(num_rows, T(0));
    for (size_t i = 0; i < values.size(); ++i) {
        result[row_indices[i]] += values[i] * vec[col_indices[i]];
    }
}

template <typename T>
void SparseMatrix<T>::multiply_transpose_with_vector(const std::vector<T> &vec, std::vector<T> &result) const {
    result.assign(num_cols, T(0));
    for (size_t i = 0; i < values.size(); ++i) {
        result[col_indices[i]] += values[i] * vec[row_indices[i]];
    }
}

// Iterator class
template <typename T>
SparseMatrix<T>::Iterator::Iterator(const SparseMatrix &matrix, size_t pos) 
    : matrix_(matrix), pos_(pos) {}

template <typename T>
bool SparseMatrix<T>::Iterator::operator!=(const Iterator &other) const {
    return pos_ != other.pos_;
}

template <typename T>
const typename SparseMatrix<T>::Iterator &SparseMatrix<T>::Iterator::operator++() {
    ++pos_;
    return *this;
}

template <typename T>
std::pair<int, int> SparseMatrix<T>::Iterator::position() const {
    return std::make_pair(matrix_.row_indices[pos_], matrix_.col_indices[pos_]);
}

template <typename T>
T SparseMatrix<T>::Iterator::value() const {
    return matrix_.values[pos_];
}

template <typename T>
typename SparseMatrix<T>::Iterator::Element SparseMatrix<T>::Iterator::operator*() const {
    return Element(matrix_.row_indices[pos_], matrix_.col_indices[pos_], matrix_.values[pos_]);
}

template <typename T>
SparseMatrix<T>::Iterator::Element::Element(int _row, int _col, T _val) 
    : row(_row), col(_col), val(_val) {}

template <typename T>
typename SparseMatrix<T>::Iterator SparseMatrix<T>::begin() const {
    return Iterator(*this);
}

template <typename T>
typename SparseMatrix<T>::Iterator SparseMatrix<T>::end() const {
    return Iterator(*this, values.size());
}

// Explicit template instantiation
template class SparseMatrix<double>;
template class SparseMatrix<float>;
