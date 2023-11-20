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
        //��left��ʼ�����ҵ��ָ����λ��
        std::string::size_type right = s.find(delimiter, left);
        //��leftһֱ���ַ���ĩβ��û�зָ��
        if (right == std::string::npos) {
            break;
        }
        //�ҵ��˷ָ��ַ����ָ�֮������������Ӧ������λ��
        *it++ = s.substr(left, right - left);
        //����������ʼλ��
        left = right + 1;
    }
    *it++ = s.substr(left);
    return i;
}

