#pragma once
#include<string>
#include<vector>
#include<atlimage.h>
class Utils
{
public:
    static unsigned int split(std::vector<std::string>& v,
        const std::string& s,
        char delimiter, unsigned int maxSegments=INT_MAX);
    static int GetImage(CImage& image, const std::string strBuffer);
};

