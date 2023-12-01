#ifndef SMPS_H
#define SMPS_H

#include <vector>
#include <string>
#include <map>

#include "sparse_matrix.h" // for SparseMatrix
#include "utils.h"         // for BijectiveMap
namespace smps
{

    class SMPSCore
    {
    public:
        // Constructor and other necessary methods

        int numRows;
        int numCols;
        BijectiveMap rowNameMap;
        BijectiveMap colNameMap;
        SparseMatrix<double> lpCoefficients; // Row-wise sparse matrix representation
        std::vector<double> costCoefficients;
        std::vector<double> rhsCoefficients;
        std::vector<char> inequalityDirections;
        std::vector<double> lowerBounds;
        std::vector<double> upperBounds;

        // Methods to read and process COR file
    };

    class SMPSTime
    {
    public:
        virtual int getStage(int index, const std::string &nameMapping) = 0;
    };

    class SMPSImplicitTime : public SMPSTime
    {
    public:
        std::vector<std::string> columnNames;
        std::vector<std::string> rowNames;

        int getStage(int index, const std::string &nameMapping) override;
    };

    class SMPSExplicitTime : public SMPSTime
    {
    public:
        std::map<std::string, int> nameToStageMap;

        int getStage(int index, const std::string &nameMapping) override;
    };

}

#endif // SMPS_H
