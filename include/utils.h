#ifndef UTILS_H
#define UTILS_H

#include <stdexcept>
#include <vector>
#include <string>
#include <map>

/**
 * BijectiveMap is a class that provides a bidirectional mapping between
 * unique names (strings) and integer indices. It allows you to associate
 * names with unique integer values and retrieve both the name associated
 * with an index and the index associated with a name.
 *
 * Assumptions:
 * - Names must not be empty.
 * - Indices start from zero and are assumed consecutive.
 * - The order in which names are added is not important.
 * - If an entry with a larger index is added, querying 'get_index' with a
 *   smaller number may lead to undefined behavior.
 */
class BijectiveMap {
public:
    /**
     * Adds a name-index pair to the mapping.
     *
     * @param name  The name to be associated with the index.
     * @param index The index to be associated with the name.
     *
     * @throws std::invalid_argument If the 'name' parameter is empty.
     * @throws std::runtime_error   If an entry with a larger index is added,
     *                             querying 'get_index' with a smaller number
     *                             may lead to undefined behavior.
     */
    void add(const std::string& name, int index);

    /**
     * Retrieves the index associated with a given name.
     *
     * @param name The name for which to retrieve the index.
     * @return The index associated with the given name.
     *
     * @throws std::runtime_error If the name is not found in the mapping.
     */
    int get_index(const std::string& name) const;

    /**
     * Retrieves the name associated with a given index.
     *
     * @param index The index for which to retrieve the name.
     * @return The name associated with the given index.
     *
     * @throws std::runtime_error If the index is not found in the mapping.
     */
    std::string get_name(int index) const;

private:
    // A map to store the mapping from names to indices.
    std::map<std::string, int> name_to_index_map;

    // A vector to store the mapping from indices to names.
    std::vector<std::string> index_to_name_vector;
};


#endif // UTILS_H
