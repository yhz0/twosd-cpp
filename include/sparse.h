#ifndef SPARSE_H
#define SPARSE_H

#include <vector>
#include <tuple>

template <typename T>
class SparseMatrix {
public:
    // Constructors
    SparseMatrix();
    SparseMatrix(const std::vector<int> &_row_indices, const std::vector<int> &_col_indices, 
                 const std::vector<T> &_values, size_t _num_rows, size_t _num_cols);

    // resize the matrix
    // this does not change the elements of the matrix
    void resize(size_t _num_rows, size_t _num_cols);

    // add an element to the matrix
    // note that duplicate elements are not checked and will cause undefined behavior
    void add_element(int row, int col, T value);

    // number of non-zeros
    size_t nnz() const;

    // get the element at (row, col) or 0 if it is not in the matrix
    T get_element(int row, int col) const;

    // multiply the matrix with a vector (as a column vector) and store the result in result
    void multiply_with_vector(const std::vector<T> &vec, std::vector<T> &result) const;

    // multiply the transpose of the matrix with a vector (as a column vector) and store the result in result
    void multiply_transpose_with_vector(const std::vector<T> &vec, std::vector<T> &result) const;

    // Iterator class
    // This class is used to iterate through the non-zero elements of the matrix
    class Iterator {
    public:
        Iterator(const SparseMatrix &matrix, size_t pos = 0);

        bool operator!=(const Iterator &other) const;
        const Iterator &operator++();
        std::pair<int, int> position() const;
        T value() const;

        struct Element {
            int row, col;
            T val;
            Element(int _row, int _col, T _val);
        };

        Element operator*() const;

    private:
        const SparseMatrix &matrix_;
        size_t pos_;
    };

    // Iterators
    Iterator begin() const;
    Iterator end() const;

private:
    size_t num_rows, num_cols;
    std::vector<int> row_indices;
    std::vector<int> col_indices;
    std::vector<T> values;
};


#endif // SPARSE_H
