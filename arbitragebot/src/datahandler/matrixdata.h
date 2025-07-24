#pragma once 

#include <vector>
#include <unordered_map>

struct MatrixData {
    std::vector<std::vector<double>> rates;
    std::unordered_map<std::string, int> currency_to_index;

    MatrixData(int size)
        : rates(size, std::vector<double>(size, 0.0))
    {}
};
