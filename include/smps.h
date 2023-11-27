#ifndef SMPS_H
#define SMPS_H

#include <string>

class SMPSTime {
public:
    bool load_time_file(const std::string& filename);
    // Other methods and attributes...
};

class SMPSSto {
public:
    bool load_sto_file(const std::string& filename);
    // Other methods and attributes...
};

#endif // SMPS_H
