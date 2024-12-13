#include "../include/socket.h"
#include "../include/macro.h"
#include "../log/log.h"
/*
    主要负责CSocket中初始化部分的代码，比如读取配置，打开监听端口等相关代码
*/
CSocket::CSocket()
{

}

CSocket::~CSocket()
{

}
bool CSocket::Initialize()
{
    if (ReadConfig())
    {
        if (OpenListeningSocket() == false)
        {
            LOG_ERROR("socket_init.cpp 中 OpenListengSocket() failed!");
            return false;
        }
        return true;
    }
    else
    {
        return true;
    }
}
bool CSocket::ReadConfig()
{
    CConfig& cfg = CConfig::Instance();
    c_listenSockCount = cfg.GetInt("ListenPortCount", 0);
    if (!c_listenSockCount) //如果监听0个端口，那肯定出问题了直接返回错误就行
    {
        LOG_ERROR("监听端口数量配置有问题!");
        return false;
    }
    return true;
}

bool CSocket::OpenListeningSocket()
{
    int                 isock;              //socket
    struct sockaddr_in  serv_addr;          //服务器的地址结构体
    int                 iport;              //端口号
    char                strinfo[100] = {0}; //临时字符串

    memset(&serv_addr, 0, sizeof(serv_addr));       //清空结构体
    serv_addr.sin_family = AF_INET;                 //选择IPV4协议族
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //监听所有IP地址(所有网卡)

    CConfig& cfg = CConfig::Instance();
    for (int i=0;i < c_listenSockCount; ++i)
    {
        isock = socket(AF_INET, SOCK_STREAM, 0);    //系统函数，成功返回非负描述符
        if (isock == -1)
        {
            LOG_ERROR("CSocket::Initialize()中socket()失败,i=%d. error:{%s}",i, std::strerror(errno));
            return false;
        }

        int reuseaddr = 1;// 为了解决time_wait状态下占用端口的问题，使用reuseaddr
        if (setsockopt(isock, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuseaddr, sizeof(reuseaddr)) == -1)
        {
            LOG_ERROR("CSocket::Initialize()中setsockopt(SO_REUSEADDR)失败,i=%d. error:{%s}",i, std::strerror(errno));
            return false;
        }
        
        int reuseport = 1;
        if (setsockopt(isock, SOL_SOCKET, SO_REUSEPORT, (const void*)&reuseport, sizeof(int)) == -1)
        {
            LOG_ERROR("CSocket::Initialize()中setsockopt(SO_REUSEPORT)失败,i=%d. error:{%s}",i, std::strerror(errno));
            //如果没处理好惊群相关的配置，可以忍受，无需返回false
        }

        if (!SetNonBlocking(isock))
        {
            LOG_ERROR("CSocket::Initialize() 中 SetNonBlocking()失败.error:{%s}", std::strerror(errno));
            close(isock);
            return false;
        }

        // 组合字符串，便于从配置项中读取相关的数据
        sprintf(strinfo, "ListenPort%d", i);
        iport = cfg.GetInt(std::string(strinfo), 10000);    //认为如果端口的配置错了，那后续客户端都连不上，再用默认也就没有意义了
        if (iport == INT_MAX)
        {
            LOG_ERROR("没找到和 %s 相关的配置项,error!", strinfo);
            close(isock);
            return false;
        }
        serv_addr.sin_port = htons((in_port_t)iport);       //in_port_t其实就是uint16_t

        //设置本服务器要监听的地址和端口
        if (bind(isock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        {
            LOG_ERROR("CSocket::Initialize()中bind()失败,i=%d. error:{%s}",i, std::strerror(errno));
            close(isock);
            return false;
        }
        // 开始监听，设置最大的队列大小为LISTEN_BACKLOG，这个和系统有关，linux会限制这个大小
        if (listen(isock, LISTEN_BACKLOG) == -1)
        {
            LOG_ERROR("CSocekt::Initialize()中listen()失败,i=%d. error:{%s}",i, strerror(errno));
            close(isock);
            return false;
        }
        c_listenSocketVec.push_back(std::make_shared<ListenItem>(iport, isock));
        LOG_INFO("监听%d端口成功", iport);
    }
    if (c_listenSocketVec.size() <= 0) // 不可能一个不监听，我之前就其实过滤了
    {
        return false;
    }
    return true;
}

// 设置非阻塞的通用代码
bool CSocket::SetNonBlocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        LOG_ERROR("CSocket 中 SetNonBlocking fcntl F_GETFL失败");
        return false;
    }
    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) == -1)
    {
        LOG_ERROR("CSocket 中 SetNonBlocking fcntl F_SETFL失败");
        return false;
    }
    return true;
}