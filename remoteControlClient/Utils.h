#pragma once
#include<string>
#include<vector>
class Utils
{
public:
    static unsigned int split(std::vector<std::string>& v,
        const std::string& s,
        char delimiter, unsigned int maxSegments=INT_MAX);
};

