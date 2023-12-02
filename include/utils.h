#ifndef UTILS_H
#define UTILS_H

#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <optional>

/**
 * BijectiveMap is a class that provides a bidirectional mapping between
 * unique names (strings) and integer indices. It uses std::optional to
 * allow querying of empty values without throwing exceptions.
 *
 * Assumptions:
 * - Names must not be empty.
 * - Indices start from zero and are assumed consecutive.
 */
class BijectiveMap {
public:
    void add(const std::string& name, int index);

    /**
     * Retrieves the index associated with a given name.
     *
     * @param name The name for which to retrieve the index.
     * @return An std::optional containing the index if found, or std::nullopt if not.
     */
    std::optional<int> get_index(const std::string& name) const;

    /**
     * Retrieves the name associated with a given index.
     *
     * @param index The index for which to retrieve the name.
     * @return An std::optional containing the name if found, or std::nullopt if not.
     */
    std::optional<std::string> get_name(int index) const;

private:
    std::map<std::string, int> name_to_index_map;
    std::vector<std::string> index_to_name_vector;
};

#endif // UTILS_H
