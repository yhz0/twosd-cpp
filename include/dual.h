#include <vector>
#include <map>

// a container that stores deduplicated float vectors of specified dimensions into a continuous memory space.
// specified number of zeros will be appended to each input vector.
// the container will not accept vectors that are already in the container.
// The container has a maximum number of vectors it can store. When it is full, the oldest vector will be replaced.
class DualVertexSet
{
public:
    // construct an empty vector set by specifying the number of zero paddings
    DualVertexSet(size_t original_dims, size_t max_size);

    // number of vectors stored in this container.
    size_t size() const;

    // get the number of original dimensions of full vector
    size_t full_original_dims() const;

    // get the number of original dimensions of full vector
    size_t full_padded_dims() const;

    // get the number of actual dimensions of short vector
    size_t short_actual_dims() const;

    // get the number of actual dimensions of short vector
    size_t short_padded_dims() const;

    // add a double vector or float vector to the end
    // returns the index of the added vector
    // if the vector is not added, returns nullptr
    size_t insert(const std::vector<float>);

    // get the address of the pos-th vector
    // no edits should be made through this pointer
    const float *get_full_dual_ptr(size_t pos) const;

    const float *get_short_dual_ptr(size_t pos) const;

private:
    size_t full_original_dims; // the dimension of the full dual vector
    size_t full_padded_dims;      // original plus the number of zero-padding for full dual vectors
    size_t short_original_dims;   // the dimension of the short dual vector
    size_t short_padded_dims;   // original plus the number of zero-padding for the short dual vector
    size_t current_size;       // number of vectors currently stored

    size_t max_size; // maximum number of vectors this container will accept.

    std::vector<float> full_duals; // the full dual vectors
    std::vector<float> short_duals; // the truncated version of the duals

    // // internal class that is used to de-duplicate a float vector using hashes
    // class Deduplicate
    // {
    // public:
    //     // hash function
    //     static unsigned int hash(const std::vector<float> &vec);

    // private:
    //     // insert an hash-index pair into the hash map
    //     bool insert(size_t index);

    //     // remove the entry of the index from the hash map
    //     void removeIndex(size_t index);

    //     // the hash of each vector corresponding to index
    //     std::multimap<unsigned int, size_t> indexHash;
    // };
};