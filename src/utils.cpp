#include "utils.h"

void BijectiveMap::add(const std::string& name, int index) {
    if (index >= static_cast<int>(indexToNameVector.size())) {
        indexToNameVector.resize(index + 1);
    }
    indexToNameVector[index] = name;
    nameToIndexMap[name] = index;
}

int BijectiveMap::getIndex(const std::string& name) const {
    auto it = nameToIndexMap.find(name);
    if (it != nameToIndexMap.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Name not found in bijection map");
    }
}

std::string BijectiveMap::getName(int index) const {
    if (index >= 0 && index < static_cast<int>(indexToNameVector.size())) {
        return indexToNameVector[index];
    } else {
        throw std::runtime_error("Index not found in bijection map");
    }
}
