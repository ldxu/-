#include "./log/log.h"
#include "./include/config.h"
#include "./include/daemon.h"

std::string configName = "./nginx.conf";
std::vector<pid_t> childList;
void FathreExit(int sig)
{
    if (!childList.empty())
    {

    
    std::cout<< "收到信号:" << sig << std::endl;
    for (pid_t& pid:childList)
        kill(pid, 2);
    }
    // for (pid_t& pid:childList)
    //     waitpid(pid,nullptr, 0 );
    std::cout<<"成功回收子进程"<<std::endl;
    exit(0) ;
}
void ChildExit(int sig)
{
    exit(0);
}
int main(int argc,const char* argv[])
{
    CConfig& cfg = CConfig::Instance();
    if (!cfg.Load(configName))
    {
        std::cout<<"配置有问题"<<std::endl;
        return 0;
    }
    std::string key = "Log";
    std::string logFolderPath = cfg.GetString(key);
    signal(SIGINT, FathreExit);
    signal(15, FathreExit);
    pid_t pid;
    switch (Daemon(pid))
    {
    case 1:
        //独立开启日志系统
        Log::Instance()->Init(1, logFolderPath.c_str(), ".log", 10);
        LOG_INFO("Start log system");
        std::cout << "我是父进程, 子进程id是: " << pid <<  std::endl;
        childList.push_back(pid);
        break;
    case 0:
        //sleep(1);
        signal(SIGINT, ChildExit);
        signal(15, ChildExit);
        Log::Instance()->Init(0, logFolderPath.c_str(), ".txt", 10);
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