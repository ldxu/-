#include "./log/log.h"
#include "./include/config.h"
#include "./include/daemon.h"
std::string configName = "./nginx.conf";

int main(int argc,const char* argv[])
{
    CConfig& cfg = CConfig::Instance();
    if (!cfg.Load(configName))
    {
        std::cout<<"配置有问题"<<std::endl;
        return 0;
    }
    std::string logFolderPath = cfg.GetString(std::string("Log"));
    Log::Instance()->Init(1, logFolderPath.c_str(), ".log", 10);
    LOG_INFO("Start log system");
    switch (Daemon())
    {
    case 1:
        std::cout << "我是父进程" << std::endl;
        break;
    case 0:
        //sleep(1);
        Log::Instance()->ReInit(0, logFolderPath.c_str(), ".txt", 10);
        LOG_INFO("Start log system");
        while(1)
        {
            sleep(2);
            LOG_INFO("我是守护进程");
        }
    default:
        std::cout<< "发生错误，直接返回" << std::endl;
        exit(0);
    }
    while(1)
    {
        for (int i=0;i<10;i++)
            LOG_INFO("async test");
        sleep(2);

    }
    sleep(30);
    std::cout << "父进程退出" << std::endl;
    return 0;
}