#include "utils.h"

void BijectiveMap::add(const std::string &name, int index)
{
    if (name.empty())
    {
        throw std::invalid_argument("Name cannot be empty");
    }

    if (index >= static_cast<int>(index_to_name_vector.size()))
    {
        index_to_name_vector.resize(index + 1);
    }
    index_to_name_vector[index] = name;
    name_to_index_map[name] = index;
}

std::optional<int> BijectiveMap::get_index(const std::string &name) const
{
    auto it = name_to_index_map.find(name);
    if (it != name_to_index_map.end())
    {
        return it->second;
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<std::string> BijectiveMap::get_name(int index) const
{
    if (index >= 0 && index < static_cast<int>(index_to_name_vector.size()))
    {
        return index_to_name_vector[index];
    }
    else
    {
        return std::nullopt;
    }
}

int BijectiveMap::size() const {
    return index_to_name_vector.size();
}