#include "../include/toolsfunc.h"
#include "../include/global.h"
#include "../include/macro.h"
#include "../log/log.h"
struct signalItem{
    int         sigNo;          //信号对应的数字编号， 每个信号都有对应的数字编号
    const char* sigName;        //信号对应的中文名字，比如SIGHUP
    void (*handler)(int signo, siginfo_t *siginfo, void* ucontext);//函数指针固定的信号处理函数模板
};


//声明信号处理函数，以后来了信号都可以在这个部分设计
static void SignalHandler(int signo, siginfo_t *siginfo, void* ucontext);
static void GetProcessStatus(void);


signalItem  signals[] ={
    // signo      signame             handler
    { SIGHUP,    "SIGHUP",           SignalHandler },        //终端断开信号，对于守护进程常用于reload重载配置文件通知--标识1
    { SIGINT,    "SIGINT",           SignalHandler },        //标识2   
	{ SIGTERM,   "SIGTERM",          SignalHandler },        //标识15
    { SIGCHLD,   "SIGCHLD",          SignalHandler },        //子进程退出时，父进程会收到这个信号--标识17
    { SIGQUIT,   "SIGQUIT",          SignalHandler },        //标识3
    { SIGIO,     "SIGIO",            SignalHandler },        //指示一个异步I/O事件【通用异步I/O信号】
    { SIGSYS,    "SIGSYS, SIG_IGN",  NULL               },        //我们想忽略这个信号，SIGSYS表示收到了一个无效系统调用，如果我们不忽略，进程会被操作系统杀死，--标识31
                                                                  //所以我们把handler设置为NULL，代表 我要求忽略这个信号，请求操作系统不要执行缺省的该信号处理动作（杀掉我）
    //...日后根据需要再继续增加
    { 0,         NULL,               NULL               }         //信号对应的数字至少是1，所以可以用0作为一个特殊标记    
};

//初始化信号的函数，用于注册信号处理程序
//返回值：0成功， -1失败
int InitSignals()
{
    signalItem* sig;                //指向自定义结构体数组的指针
    struct sigaction sa;            //sigaction：系统定义的信号相关的结构，用来注册不同的信号函数

    //正如在结构体数组中写的那样，专门有一个0表示定义信号的结尾
    for (sig = signals; sig->sigNo != 0; ++sig)
    {
        memset(&sa, 0, sizeof(struct sigaction));

        if (sig->handler)//如果有定义专门的信号处理函数，那么就可以进行注册，否则忽略
        {
            sa.sa_sigaction = sig->handler;
            sa.sa_flags = SA_SIGINFO;           //设置此标志意味着处理函数的原型可以是void handler(int, siginfo_t *, void *)可以获得更丰富的信息
        }
        else
        {
            sa.sa_handler = SIG_IGN;            //如果没有专门配置信号处理函数，那么就直接忽视
        }

        sigemptyset(&sa.sa_mask);               //将信号屏蔽集清空，表示不屏蔽任何信号

        //设置信号处理函数
        if (sigaction(sig->sigNo, &sa, NULL) == -1)
        {
            LOG_ERROR("_signal.cpp 中 sigaction(%s) failed", sig->sigName);
            return -1;
        }
        else
        {
            LOG_INFO("成功配置 %s 的信号处理函数!", sig->sigName);
        }
    }
    return 0;
}

// 信号处理函数的定义
static void SignalHandler(int signo, siginfo_t *siginfo, void* ucontext)
{
    signalItem*     sig;            //指向自定义的信号数组
    char*           action;         //记录信号的字符串

    for (sig = signals; sig->sigNo != 0; ++sig)
    {
        if (sig->sigNo == signo)
            break;
    }

    action = (char*)"Terminal";          //无法获知发送信号的进程id，则把他归为匿名

    if (g_processStatuCode == PROCESS_MASTER) // 如果是master进程则执行这一套的信号逻辑
    {
        switch (signo)
        {
            case SIGCHLD:
                g_childReraise = 1;
                break;
            case SIGTERM:
                g_masterProcExitCode = 1;           //父进程退出标志
                break;
            /*
                其他的信号处理需要的操作
            */
            default:
                break;
        }
    }
    else if(g_processStatuCode == PROCESS_WORKER)// worker进程执行的信号处理逻辑
    {
        /*
            worker进程执行的信号处理函数，后续想到了要加什么再完善
        */
        switch (signo)
        {
            case SIGTERM:
                g_workerProcExitCode = 1;           //父进程退出标志
                break;
            /*
                其他的信号处理需要的操作
            */
            default:
                break;
        }
    }
    else//其他状态的信号来了，暂且不处理
    {

    }

    // 记录一些日志信息
    if (siginfo && siginfo->si_pid)
    {
        LOG_INFO("signal %d (%s) received from %d", signo, sig->sigName, siginfo->si_pid);
    }
    else
    {
        LOG_INFO("signal %d (%s) received from %s", signo, sig->sigName, action);
    }

    // 如果是子进程状态变化，那么需要捕捉子进程的变化
    if (signo == SIGCHLD)
    {
        GetProcessStatus(); // 获取子进程的结束状态
    }

    return;

}
// 获取子进程的结束状态，防止单独kill子进程时，子进程变成僵尸进程
static void GetProcessStatus(void)
{
    pid_t       pid;
    int         status;
    int         err;
    int         one = 0;    //标记信号正常处理过一次   

    // 处理子进程的SIGCHLD信号
    for (;;)
    {
        pid = waitpid(-1, &status, WNOHANG);    //第一个参数为-1，表示等待任何的子进程 
                                                //第二参数可以用来查询子进程退出的一些状态信息等
        if (pid == 0)   //因为非阻塞状态，理论上发生了SIGCHLD信号，可以回收子进程的结构体，0那就直接返回
        {
            return;
        }

        if (pid == -1) // waitpid调用出现错误
        {
            err = errno;    //错误状态码，用于后续的记录
            if (err = EINTR)
            {
                continue;   //如果是因为被其他信号终止，那么等待下次执行
            }

            if (err == ECHILD && one)
            {
                return;//如果是没有子进程了，找不到，并且之前处理过了，直接返回
            }

            if (err == ECHILD)  //没有子进程
            {
                LOG_ERROR("_siginal.cpp 中 waitpid() failed! error:{%s}", std::strerror(err));
                return;
            }
            //应该用于警告，而不是错误
            LOG_ERROR("_siginal.cpp 中 waitpid() failed! error:{%s}", std::strerror(err));
            return;
        }
        //走到这里就是成功处理了
        one = 1;    //记录已经处理了一次信号
        if (WTERMSIG(status))
        {
            LOG_ERROR("pid = %d exited on signal %d!",pid, WTERMSIG(status)); //获取使子进程终止的信号编号
        }
        else
        {
            LOG_ERROR("pid = %d exited with code %d!",pid,WEXITSTATUS(status));
        }
    }    
    return;      
}

