#include "_signal.h"
#include "toolsfunc.h"

int Daemon(pid_t& m_pid)
{
    //step1 fork出子进程 如果不想被干扰，则可以先修改信号集，保证守护进程的创建过程不被干扰
    //BlockSignals();
    pid_t pid;
    switch ((pid = fork()))
    {
    case -1:
        //LOG_ERROR("Deamon() 中 fork () 失败, error:{%s}", strerror(errno));
        return -1;
    case 0:
        // 子进程流程退出执行后续操作
        break;
    default:
        //父进程需要直接返回 同时恢复之前的信号集
        m_pid = pid;
        //UnBlockSignals();
        return 1;
    }

    //子进程流程走到了这里
    //step2 脱离终端，成为新的会话首领
    if (setsid() == -1)
    {
        //LOG_ERROR("Daemon() 中 setsid() 失败, error:{%s}", strerror(errno));
        return -1;
    }
    //step3 给守护进程最大的文件权限，避免没必要的麻烦
    umask(0);

    //step4 打开黑洞设备，把守护进程的输入和输出重定向到黑洞设备

    int fd = open("/dev/null", O_RDWR);
    if (fd == -1)
    {
        //LOG_ERROR("Daemon() 中打开黑洞设备失败, error:{%s}", strerror(errno));
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) == -1)
    {
        //LOG_ERROR("Daemon() 中dup2(STDIN_FILENO)失败, error:{%s}", strerror(errno));
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        //LOG_ERROR("Daemon() 中dup2(STDOUT_FILENO)失败, error:{%s}", strerror(errno));
        return -1;
    }

    if (fd > STDERR_FILENO)
    {
        if (close(fd) == -1)
        {
            //LOG_ERROR("Daemon() 中close(fd)失败, error:{%s}", strerror(errno));
            return -1;
        }
    }
    //正常退出，恢复信号集
    //UnBlockSignals();
    return 0;

}