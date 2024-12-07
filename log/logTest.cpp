#include "log.h"
int main()
{
    Log::Instance();
    Log::Instance()->init(1,"./log", ".log", 10);
    int a = 10;
    float b = 12.5;
    const char str[] = "china china";
    LOG_BASE(3, "%s 11111111", str)
    LOG_INFO("开启了异步日志");
    for (int i=0;i<30;i++)
    {
        sleep(1);
        LOG_BASE(3, "%s 11111111", str)
    }
    return 0;
}