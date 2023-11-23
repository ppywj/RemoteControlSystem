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
//返回1--内存不足
//返回2--创建全局流对象失败
//返回0--正常
int Utils::GetImage(CImage& image, const std::string strBuffer)
{
    IStream* pStream = NULL;
    BYTE* pData = (BYTE*)strBuffer.c_str();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMem == NULL)
    {
        TRACE("内存不足\r\n");
        Sleep(1);
        return 1;
    }
    HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
    if (hRet == S_OK) {
        ULONG length = 0;
        pStream->Write(pData, strBuffer.size(), &length);
        image.Load(pStream);
        return 0;
    }
    else
    {
        return 2;
    }
}

