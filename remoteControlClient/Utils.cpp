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
//����1--�ڴ治��
//����2--����ȫ��������ʧ��
//����0--����
int Utils::GetImage(CImage& image, const std::string strBuffer)
{
    IStream* pStream = NULL;
    BYTE* pData = (BYTE*)strBuffer.c_str();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMem == NULL)
    {
        TRACE("�ڴ治��\r\n");
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

