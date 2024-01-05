#include "vector_container.h"
#include <algorithm>
#include <cstring>

VectorContainer::VectorContainer(size_t max_vecs, size_t vec_dim) 
    : max_vectors(max_vecs), vector_dim(vec_dim), 
      padded_vector_dim(calculate_padded_dim(vec_dim)), 
      current_size(0), current_position(0), 
      sync_start_position(0), wrap_around_flag(false),
      full_flag(false) {
    data_storage = new float[max_vectors * padded_vector_dim]();
}


VectorContainer::~VectorContainer() {
    delete[] data_storage;
}

VectorContainer::VectorContainer(VectorContainer&& other) noexcept
    : max_vectors(other.max_vectors), vector_dim(other.vector_dim),
      padded_vector_dim(other.padded_vector_dim), current_size(other.current_size),
      current_position(other.current_position), sync_start_position(other.sync_start_position),
      wrap_around_flag(other.wrap_around_flag), data_storage(other.data_storage) {
    other.data_storage = nullptr;
    other.current_size = 0;
}

VectorContainer& VectorContainer::operator=(VectorContainer&& other) noexcept {
    if (this != &other) {
        delete[] data_storage;

        max_vectors = other.max_vectors;
        vector_dim = other.vector_dim;
        padded_vector_dim = other.padded_vector_dim;
        current_size = other.current_size;
        current_position = other.current_position;
        sync_start_position = other.sync_start_position;
        wrap_around_flag = other.wrap_around_flag;
        data_storage = other.data_storage;

        other.data_storage = nullptr;
        other.current_size = 0;
    }
    return *this;
}

std::optional<size_t> VectorContainer::insert(const std::vector<float>& vec) {
    if (vec.size() != vector_dim) return std::nullopt;
    return pad_and_store(vec.data());
}

std::optional<size_t> VectorContainer::insert(const std::vector<double>& vec) {
    if (vec.size() != vector_dim) return std::nullopt;
    std::vector<float> temp(vec.begin(), vec.end());
    return insert(temp);
}

std::optional<size_t> VectorContainer::insert(const double* vec) {
    std::vector<double> temp(vec, vec + vector_dim);
    return insert(temp);
}

size_t VectorContainer::get_vector_dims() const {
    return vector_dim;
}

size_t VectorContainer::get_padding_dims() const {
    return padded_vector_dim - vector_dim;
}

size_t VectorContainer::size() const {
    return current_size;
}

std::vector<float> VectorContainer::get(size_t index) const {
    std::vector<float> vec(vector_dim);
    if (index < current_size) {
        std::memcpy(vec.data(), &data_storage[index * padded_vector_dim], vector_dim * sizeof(float));
    }
    return vec;
}

const float* VectorContainer::data() const {
    return data_storage;
}

size_t VectorContainer::get_current_position() const
{
    return current_position;
}

size_t VectorContainer::get_sync_position() const
{
    return sync_start_position;
}

bool VectorContainer::get_wrap_around_flag() const
{
    return wrap_around_flag;
}

size_t VectorContainer::pad_and_store(const float* src) {
    size_t offset = current_position * padded_vector_dim;
    std::memcpy(&data_storage[offset], src, vector_dim * sizeof(float));

    // record the position where the vector is actually stored
    size_t pos = current_position;
    
    if (current_size < max_vectors) ++current_size;

    current_position ++;
    if (current_position >= max_vectors) {
        wrap_around_flag = true;
        current_position = 0;
    }

    // if looped over once, and we are back at the record position,
    // then we need to update the entire vector
    // once it is full, we would also need to update the record position
    // appropriately
    if (wrap_around_flag && current_position == sync_start_position)
    {
        full_flag = true;
    }

    if (full_flag)
    {
        sync_start_position = current_position;
    }

    return pos;
}

void VectorContainer::reset_sync_range() {
    sync_start_position = current_position;
    wrap_around_flag = false;
    full_flag = false;
}

size_t VectorContainer::calculate_padded_dim(size_t dim) const {
    return (dim + GROUP_SIZE - 1) / GROUP_SIZE * GROUP_SIZE;
}

std::optional<size_t> UniqueVectorContainer::insert(const std::vector<float> &vec)
{
    if (vec.size() != vector_dim) return std::nullopt;
    // calculate the hash value
    float hash_value = hash(vec);
    
    if (hashmap.find(hash_value) != hashmap.end())
    {
        std::vector<size_t> &indices = hashmap[hash_value];
        for (size_t i = 0; i < indices.size(); ++i)
        {
            // if we find a vector that is the same as the input vector
            // then dont insert it
            if (approx_equal(vec, get(indices[i])))
            {
                return std::nullopt;
            }
        }
    }

    // if we reach here, then we need to insert the vector

    // check if we need to overwrite the oldest vector,
    // if overwriting, we should also remove the old index from the hashmap
    if (current_size == max_vectors)
    {
        float oldest_hash = hash(get(current_position));
        std::vector<size_t> &indices = hashmap[oldest_hash];
        auto it = std::find(indices.begin(), indices.end(), current_position);
        if (it != indices.end())
        {
            indices.erase(it);
        }
    }

    // add the hash value to the hashmap
    if (hashmap.find(hash_value) == hashmap.end())
    {
        hashmap[hash_value] = std::vector<size_t>();
    }
    hashmap[hash_value].push_back(current_position);

    // store the vector
    return VectorContainer::insert(vec);
}

std::optional<size_t> UniqueVectorContainer::insert(const std::vector<double> &vec) {
    std::vector<float> temp(vec.begin(), vec.end());
    return insert(temp);
}

std::optional<size_t> UniqueVectorContainer::insert(const double *vec)
{
    std::vector<float> temp(vec, vec + vector_dim);
    return insert(temp);
}

float UniqueVectorContainer::hash(const std::vector<float> &vec)
{
    float hash = 0;
    for (size_t i = 0; i < vec.size(); ++i)
    {
        hash += std::abs(vec[i]);
    }
    return hash;
}

bool UniqueVectorContainer::approx_equal(const std::vector<float> &v1, const std::vector<float> &v2)
{
    for(size_t i = 0; i < v1.size(); ++i)
        if(std::abs(v1[i] - v2[i]) > TOLERANCE)
            return false;
    return true;
}
