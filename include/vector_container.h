#include <vector>
#include <optional>
#include <cstddef>

class VectorContainer {
public:
    static constexpr size_t GROUP_SIZE = 8;

    // Constructor
    VectorContainer(size_t max_vectors, size_t vector_dim);

    // Destructor
    ~VectorContainer();

    // Disallow Copying
    VectorContainer(const VectorContainer&) = delete;
    VectorContainer& operator=(const VectorContainer&) = delete;

    // Enable Moving
    VectorContainer(VectorContainer&& other) noexcept;
    VectorContainer& operator=(VectorContainer&& other) noexcept;

    // Insert operations
    // WARNING: insertion is not thread safe!
    std::optional<size_t> insert(const std::vector<float>& vec);
    std::optional<size_t> insert(const std::vector<double>& vec);
    std::optional<size_t> insert(const double* vec);

    // Accessors
    size_t get_vector_dims() const;
    size_t get_padding_dims() const;
    size_t size() const;
    std::vector<float> get(size_t index) const;
    const float* data() const;

    // Sync related functions
    size_t get_current_position() const;
    size_t get_sync_position() const;
    bool get_wrap_around_flag() const;

    void reset_sync_range();

private:
    size_t max_vectors;
    size_t vector_dim;
    size_t padded_vector_dim;

    size_t current_size;
    size_t current_position;

    // sync related
    size_t sync_start_position;
    bool wrap_around_flag;
    bool full_flag;

    float* data_storage;

    // Helper functions
    size_t pad_and_store(const float* src);
    size_t calculate_padded_dim(size_t dim) const;
};
