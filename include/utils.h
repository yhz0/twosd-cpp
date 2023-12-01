#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <map>
#include <stdexcept>

class BijectiveMap {
public:
    void add(const std::string& name, int index);
    int getIndex(const std::string& name) const;
    std::string getName(int index) const;

private:
    std::map<std::string, int> nameToIndexMap;
    std::vector<std::string> indexToNameVector;
};

#endif // UTILS_H
