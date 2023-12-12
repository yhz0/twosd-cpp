#include <vector>

// a container that stores deduplicated float vectors of specified dimensions into a continuous memory space.
// an element of value 1.0f will be prepended into the vector, and specified number of zeros will be appended to each input vector
class DualVertexSet
{
public:
    // create an empty container
    DualVertexSet(size_t original_dims);

    // specify
    DualVertexSet(int original_dims, int zero_pad_dims);

    // number of vectors stored in this container.
    size_t get_length();

    // get the number of original dimensions of each vector
    // note: to read the entire vector, actual_dims+1 elements should be read
    size_t get_actual_dims();

    // get the number of actual dimensions of each vector
    size_t get_actual_dims();

    // add a double vector or float vector to the end
    // returns the address of the added vector
    // if the vector is not added, returns nullptr
    // FIXME: how to accept single vector as well?
    float *insert(const std::vector<float>);

    // get the address of the pos-th vector
    // no edits should be made through this pointer
    const float *get_address(size_t pos);

private:
    // the dimension of the input vector
    size_t original_dims;

    // the number of zero paddings at the end of the vector
    size_t zero_pad_dims;

    // the dimension of the vector in duals
    size_t actual_dims;

    // number of vectors currently stored
    size_t current_length;

    // the actual container that stores the vertex
    std::vector<float> duals;
};
