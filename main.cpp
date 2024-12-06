#include "./log/log.h"
#include "./include/config.h"

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
    Log::Instance()->init(1, logFolderPath.c_str());
    LOG_INFO("Start log system");
    return 0;
}