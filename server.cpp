#include "./log/log.h"
#include "./include/global.h"
#include "./include/config.h"
#include "./include/toolsfunc.h"
#include "./include/macro.h"
#include "./include/socket.h"
/*
    内存检测工具的使用：valgrind --leak-check=full --leak-check=full --show-leak-kinds=all ./Server
    g_  外部全局变量
    c_  类内变量
    m_  全局变量
    _\  局部变量
*/

//---------------------------------------------------全局变量声明 START------------------------------------------------------
int     g_os_argc;                      //参数个数
char    **g_os_argv;                    //命令行参数数组首地址
int     g_daemonized = 0;               //守护进程标记，标记是否启用了守护进程模式
int     g_processStatuCode;             //进程标记
int     g_stopEvent;                    //进程退出标志
const char* configName = "./nginx.conf";//日志文件地址
CSocket g_socket;                       //全局的通信对象
pid_t   m_procPid;                      //保存当前进程的id
sig_atomic_t  g_childReraise;           //子进程退出的标记，master进程需要重新拉起一个子进程
//---------------------------------------------------全局变量声明 END------------------------------------------------------
/*
    std::cerr << "\033[31m" << "This is an error message in red" << "\033[0m" << std::endl;
*/
int main(int argc, char* argv[])
{
    int _exitCode = 0;          //退出代码
    // 正常的配置项加载过程，文件句柄由析构函数负责关闭
    CConfig& cfg = CConfig::Instance();
    if (!cfg.Load(configName))
    {
        std::cerr << "\033[31m" << "读取配置文件出错!程序退出。" << "\033[0m" << std::endl;
        return 0;
    }
    // 决定早点就启用日志，这样可以记录更多的信息
    // 从配置文件中读取日志文件的存放地址
    std::string _logFolderPath = cfg.GetString("Log");
    // master进程使用同步日志就行
    Log::Instance()->Init(1,_logFolderPath.c_str(), ".log", 0);
    pid_t _parent = getpid();

    // 开机显示
    LOG_INFO("  X   X   L        DDDD  ");
    LOG_INFO("   X X    L        D   D ");
    LOG_INFO("    X     L        D   D ");
    LOG_INFO("   X X    L        D   D ");
    LOG_INFO("  X   X   LLLLLLL  DDDD   ");
    LOG_INFO("启动程序, 进程编号为pid:%d", _parent);
    // 保留 argv 以及 argv 数组的指针，方便子进程根据这两个信息来修改进程名称 
    g_os_argc = argc;   //保存参数的个数
    g_os_argv = (char **) argv;

    int _daemonCode = cfg.GetInt("Daemon", 0);

    g_processStatuCode = PROCESS_MASTER;
    m_procPid = _parent;
    pid_t _daemon;
    // 信号初始化，不同的类型进程对于信号的处理是不一样的
    if (InitSignals() !=0 )
    {
        LOG_ERROR("Server.cpp 中 InitSignals() failed!");
        _exitCode = 1;
        goto lblexit;
    }

    // Socket对象初始化（打开监听端口）
    if (g_socket.Initialize() == -1)
    {
        LOG_ERROR("Server.cpp 中 g_socket.Initialize() failed!");
        _exitCode = 1;
        goto lblexit;
    }

    // 以守护进程的方式运行
    if (_daemonCode == 1)
    {
        LOG_INFO("检测到开启守护进程...")
        // 运行守护进程程序
        int _daemonStateCode = Daemon(_daemon);
        if (_daemonStateCode == -1)// 创建守护进程出问题，那么认为可以直接结束程序
        {
            LOG_INFO("创建守护进程过程中出现错误, Daemon() failed");
            _exitCode = 1;
            goto lblexit;
        }

        if (_daemonStateCode == 1)//父进程返回，那么父进程可以直接出去了
        {
            _exitCode = 0;
            LOG_INFO("进程[pid:%d]成功创建守护进程[pid:%d]", _parent, _daemon);
            goto lblexit;
        }
        g_daemonized = 1;               //守护进程的标记
        m_procPid = getpid();           //记录守护进程的ID
    }

    MainProcessCycle();
    

lblexit:
    if (g_daemonized)
    {
        LOG_INFO("守护进程[pid:%d]退出----------------------------------------------", m_procPid);
    }
    else
    {
        LOG_INFO("进程[pid:%d]退出----------------------------------------------", m_procPid);
    }
    return _exitCode;
}

