#include "pch.h"
#include "Utils.h"
#include <iterator>


unsigned int Utils::split(std::vector<std::string>& v,
    const std::string& s,
    char delimiter, unsigned int maxSegments) {
    v.clear();
    std::back_insert_iterator<std::vector<std::string> > it(v);
    std::string::size_type left = 0;
    unsigned int i;
    for (i = 1; i < maxSegments; i++) {
        //从left开始往后找到分割符的位置
        std::string::size_type right = s.find(delimiter, left);
        //从left一直到字符串末尾都没有分割符
        if (right == std::string::npos) {
            break;
        }
        //找到了分割字符，分割之后存入迭代器对应的容器位置
        *it++ = s.substr(left, right - left);
        //更新搜索起始位置
        left = right + 1;
    }
    *it++ = s.substr(left);
    return i;
}

