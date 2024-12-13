
/**
 * @description:一个工具集源文件
 * @time:2024.12.6
 */
#include "../include/toolsfunc.h"

// char* Ltrim(char* str, const char cc)
// {
//     if (str==nullptr || *str=='\0')
//         return nullptr;
    
//     char* p = str;
//     while (*p==cc)
//         p++;
//     memmove(str, p, strlen(str) - (p - str) + 1);
//     return str;
// } 

std::string& Ltrim(std::string& str, const char cc )
{
    auto pos = str.find_first_not_of(cc);   //从左往右找到第一个不是cc的字符的位置，没找到则是0
    if (pos != 0) str.replace(0, pos, "");
    return str;
}

// char* Rtrim(char* str, const char cc)
// {
//     if (str==nullptr || *str=='\0')
//         return nullptr;
    
//     size_t len = strlen(str);
//     while (len>0 && str[len-1]==cc)
//         --len;
//     str[len] = '\0';
//     return str;
// }

std::string& Rtrim(std::string& str, const char cc)
{
    auto pos = str.find_last_not_of(cc);
    if (pos != std::string::npos)
        str.erase(pos + 1);
    return str;
}

std::vector<std::string>& Split(const std::string& str, std::vector<std::string>& dest, const std::string& delimiter)
{
    if (str.length() == 0 || delimiter.length() == 0)
        return dest;
    size_t  start = 0, end = 0;
    while ( (end = str.find(delimiter, start)) != std::string::npos)
    {
        dest.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    dest.push_back(str.substr(start));
    return dest;
}

std::vector<std::string> Split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> temp;
    Split(str, temp, delimiter);
    return temp;
}
