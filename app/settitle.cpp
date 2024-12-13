#include "../include/toolsfunc.h"
#include "../include/global.h"
// 使用指向指针的指针来操作main函数中的那个全局变量
void SetProcessName(const char* title, int argc, char* argv[])
{
    size_t _totalArgvLength = 0;
    for (int i=0; i < argc; ++i)
    {
        _totalArgvLength += strlen(argv[i]) + 1;
    }

    size_t _titleNameLength = strlen(title);
    if (_titleNameLength > _totalArgvLength)
    {
        std::cerr << "\033[31m" << "Warning: New process name is too long, it will be truncate."  << "\033[0m" << std::endl;
        _titleNameLength = _totalArgvLength;
    }

    memset(argv[0], 0, _totalArgvLength);       //清空原来的 argv 空间
    strncpy(argv[0], title, _titleNameLength);  //复制新的名称到 argv[0]

    for (int i=1; i < argc; ++i)
    {
        argv[i] = nullptr;
    }

    // #ifdef __linux__
    // if (prctl(PR_SET_NAME, title, 0, 0, 0)!=0)
    // {
    //     std::perror("prctl failed");
    // }
    // #endif
}