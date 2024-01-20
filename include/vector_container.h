#include <vector>
#include <optional>
#include <cstddef>
#include <unordered_map>

class VectorContainer
{
public:
    static constexpr size_t GROUP_SIZE = 8;

    // Constructor
    VectorContainer(size_t max_vectors, size_t vector_dim);

    // Destructor
    ~VectorContainer();

    // Disallow Copying
    VectorContainer(const VectorContainer &) = delete;
    VectorContainer &operator=(const VectorContainer &) = delete;

    // Enable Moving
    VectorContainer(VectorContainer &&other) noexcept;
    VectorContainer &operator=(VectorContainer &&other) noexcept;

    // Insert operations
    // WARNING: insertion is not thread safe!
    virtual std::optional<size_t> insert(const std::vector<float> &vec);
    virtual std::optional<size_t> insert(const std::vector<double> &vec);
    virtual std::optional<size_t> insert(const double *vec);

    // Accessors
    size_t get_vector_dims() const;
    size_t get_padding_dims() const;
    size_t size() const;
    std::vector<float> get(size_t index) const;
    const float *data() const;

    // Sync related functions
    size_t get_current_position() const;
    size_t get_sync_position() const;
    bool get_wrap_around_flag() const;

    void reset_sync_range();

protected:
    size_t max_vectors;
    size_t vector_dim;
    size_t padded_vector_dim;
    size_t pad_and_store(const float *src);

    size_t current_size;
    size_t current_position;

private:
    // sync related
    size_t sync_start_position;
    bool wrap_around_flag;
    bool full_flag;

    float *data_storage;

    size_t calculate_padded_dim(size_t dim) const;
};

class UniqueVectorContainer : public VectorContainer
{
public:
    // Inherit constructors
    using VectorContainer::VectorContainer;

    static constexpr float TOLERANCE = 1e-6;
    std::optional<size_t> insert(const std::vector<float> &vec) override;
    std::optional<size_t> insert(const std::vector<double> &vec) override;
    std::optional<size_t> insert(const double *vec) override;

private:
    // map from hash to indices
    std::unordered_map<float, std::vector<size_t>> hashmap;

    // produce a hash from a vector, using the 1-norm
    static float hash(const std::vector<float> &vec);

    // compare two vectors
    static bool approx_equal(const std::vector<float> &v1, const std::vector<float> &v2);
};