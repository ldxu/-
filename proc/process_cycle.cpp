#include "../include/toolsfunc.h"
#include "../include/global.h"
#include "../include/config.h"
#include "../include/macro.h"
#include "../log/log.h"

//用于创建worker进程
static void StartWorkerProcess(int processCount, std::unordered_map<pid_t, std::string>& umap);
//创建每个worker进程的函数
static int  SpawnProcess(int inum, const char* procName, const char* logFolderPath);
//worker进程进入的循环
static void WorkerProcessCycle(int inum, const char* procName);
void MainProcessCycle()
{
    sigset_t    set;        //声明一个信号集
    sigemptyset(&set);      //清空信号集，如果这个时候使用，那么不会屏蔽任何信号

    //在执行本函数期间，不希望收到的信号（这个阶段需要创建worker进程了，不希望被中断）
    sigaddset(&set, SIGCHLD);     //子进程状态改变
    sigaddset(&set, SIGALRM);     //定时器超时
    sigaddset(&set, SIGIO);       //异步I/O
    sigaddset(&set, SIGINT);      //终端中断符
    sigaddset(&set, SIGHUP);      //连接断开
    sigaddset(&set, SIGUSR1);     //用户定义信号
    sigaddset(&set, SIGUSR2);     //用户定义信号
    sigaddset(&set, SIGWINCH);    //终端窗口大小改变
    sigaddset(&set, SIGTERM);     //终止
    sigaddset(&set, SIGQUIT);     //终端退出符
    //.........可以根据开发的实际需要往其中添加其他要屏蔽的信号......

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1)
    {
        LOG_ERROR("MainProcessCycle 中 sigprocmaks() failed. error:{%s}", std::strerror(errno));
    }

    //设置进程的标题master/worker
    SetProcessName("Master", g_os_argc, g_os_argv);
    LOG_INFO("【Master】进程启动...");

    int workProcess = CConfig::Instance().GetInt("WorkerProcesses", 1);
    std::unordered_map<pid_t, std::string>  workerProcessUmap;  //记录进程号和log文件的对应关系，后续如果worker进程挂了，重新启动，并且延用之前的日志

    StartWorkerProcess(workProcess, workerProcessUmap);
    sigemptyset(&set);  //清空信号屏蔽字，这个时候可以接受信号了 

    for (;;)
    {
        sigsuspend(&set);           //阻塞在这里，等待一个信号，此时进程是挂起的，不占用cpu时间s
        /*
            预计在这里加上检测子进程退出后，重新将子进程启动,此时就需要一个标志位来记录行为了
            
        */
       std::cerr << "Master进程检测到信号" << std::endl;
       // 重新启动worker进程
       if (g_childReraise)
       {
        std::cerr << "Master进程开始检测子进程的状态" << std::endl;
        loopRaiseChild:
            //如果子进程退出去了，那么重新拉起来一个子进程
            for (auto& it:workerProcessUmap)
            {
                switch (kill(it.first, 0))
                {
                case 0:
                    //如果进程存在则跳出即可
                    break;
                case -1:
                    if (errno == ESRCH)
                    {
                        pid_t pid = SpawnProcess(0, "Worker", it.second.c_str());
                        if (pid == -1)
                        {
                            LOG_ERROR("检测到worker进程 %d 退出, 重新拉起进程失败!", it.first, pid);
                        }
                        else if(pid > 0)
                        {
                            LOG_INFO("检测到worker进程 %d 退出, 重新拉起进程[%d]!", it.first, pid)
                            workerProcessUmap[pid] = it.second;
                            workerProcessUmap.erase(it.first);
                            //因为是重新插入进去的，所以需要再次检测是不是还有进程挂了
                            goto loopRaiseChild;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        //做完了重新拉起子进程的操作需要把标志位归位
        g_childReraise = 0;
       }
    }
    return;
}

static void StartWorkerProcess(int processCount, std::unordered_map<pid_t, std::string>& umap)
{
    for (int i=0; i < processCount; ++i)
    {
        //不保证全部都创建成功
        char logPath[100] = {0};
        sprintf(logPath, "%s/worker%d/", CConfig::Instance().GetString("Log").c_str(), i);
        pid_t pid = SpawnProcess(i, "Worker", logPath);
        switch (pid)
        {
        case -1:
            //如果没创建成功，那么直接退出循环，创建下一个就行
            break;
        case 0:
            //子进程也要退出来了才可能到这也是直接退
            break;
        default:
            //父进程返回，带回来了worker进程的id，需要记录
            umap[pid] = std::string(logPath);
            LOG_INFO("启动Worker进程[%d], 日志记录地址为:%s", pid, logPath);
            break;
        }
    }
    return;
}


static int  SpawnProcess(int inum, const char* procName, const char* logFolderPath)//用于创建worker进程
{
    pid_t   pid;
    pid = fork();
    switch (pid)
    {
    case -1:
        LOG_ERROR("SpawnProcess() 中 fork() failed! error:{%s}", std::strerror(errno));
        break;
    case 0:
        //子进程进入到这里
        g_procPid = getpid();   //保留进程号信息
        SetProcessName(procName, g_os_argc, g_os_argv);     //设置新标题
        Log::Instance()->Init(1, logFolderPath, ".log", 1024);  //开启独立日志系统
        WorkerProcessCycle(inum, procName);
        break;
    default:
        break;
    }
    return pid;
}

static void WorkerProcessCycle(int inum, const char* procName)
{
    LOG_INFO("Worker进程[%d]开始工作！", getpid());
    g_processStatuCode = PROCESS_WORKER;
    sleep(10);
    exit(0);
}
