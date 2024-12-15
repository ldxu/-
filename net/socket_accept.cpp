#include "../include/socket.h"

/// @brief 处理用户连接事件，和监听套接字相关
/// @param _Conn 一个连接的共享只能指针
void CSocket::AcceptHandler(ConnectionItem* _Conn)
{
    struct sockaddr     mysockaddr;         //远端服务器的sock地址
    socklen_t           socklen;            //保存结构体大小
    int                 err;
    int                 level;              //保存errno错误码
    int                 connectFd;          //新连接用户的套接字
    static int          use_accept4 = 1;    //使用accept4()函数标记s
    std::shared_ptr<ConnectionItem> newConn;//用来接收连接池中连接的智能指针

    socklen = sizeof(mysockaddr);
    do
    {
        if (use_accept4)
        {
            connectFd = accept4(_Conn->c_fd,&mysockaddr, &socklen, SOCK_NONBLOCK);
        }
        else
        {
            connectFd = accept(_Conn->c_fd, &mysockaddr, &socklen);
        }

        if (connectFd == -1)
        {
            err = errno;
            if (err = EAGAIN) //EAGAIN 可以理解为，没得读了，再来一遍，那么就可以退出去了
            {
                // 先不做，后面会统一退出
            }

            if (err = ECONNABORTED)
            {
                //这个经常出异常，但是不影响连接，就先不加上
                //LOG_WARN("AcceptHandler 中发生了意外事件.->{%s}", std::strerror(err));
            }
            else if ( err == EMFILE || err == ENFILE) // ENFILE表明进程中的fd用完了，也就是套接字资源耗尽
            {
                LOG_WARN("在接收用户连接的过程中发现进程的套接字资源耗尽了，错误码 %d {%s}", err, std::strerror(err));
                // 都耗尽了还有必要进入循环继续读吗？
            }

            if (use_accept4 && err == ENOSYS)    //ENOSYS请求的系统调用没有实现，那就是说这个函数失效了
            {
                LOG_WARN("use_accept4() 失效，请注意!");
                continue;   //继续去拉取用户请求
            }

            if (err == ECONNABORTED) // 对方已经关闭套接字了，啥也不用做了
            {

            }

            break;

        }

        //到了这一步。那就是正常获取到了连接了
        //在线用户数大于我们所设置的连接池数量
        // if (c_onlineUsers >= c_workerConnections)    //没有可用的连接了，得拒绝
        // {
        //     LOG_WARN("超出系统允许的最大连入用户数(%d), 拒绝连接请求！", c_totalConnections);
        //     close(connectFd);   //关闭拉上来的连接
        //     break;
        // }
        if (c_freeWorkerConnections <= 0)
        {
            LOG_WARN("没有空闲连接了, 拒绝用户连接请求!");
            close(connectFd);   //关闭拉上来的连接
            break;
        }
        // 暂且设定，没有空闲连接了就直接拒绝

        newConn = GetConnection(connectFd);

        if (newConn == nullptr)
        {
            close(connectFd);   //没有空闲连接分配，关闭，但是不会出现这个问题，我没做其他限制
            break;
        }

        memcpy(&newConn->c_clientSock, &mysockaddr, socklen);       //拷贝客户端地址到连接对象

        if (!use_accept4)
        {
            if (SetNonBlocking(connectFd) == false)
            {
                CloseConnection(newConn);
                break;
            }
        }

        newConn->c_listen = _Conn->c_listen;                    //连接对象和监听对象绑定，但是这个难道不是conenct -> listen?
        newConn->c_writeHandler = &CSocket::WriteRequestHandler;
        newConn->c_readHandler = &CSocket::ReadRequestHandler;
        if (EpollOperEvent(
            connectFd,
            EPOLL_CTL_ADD,
            EPOLLIN | EPOLLRDHUP | EPOLLET,
            0,
            newConn
        ) == -1)
        {
            CloseConnection(newConn);   //如果不能添加新的感兴趣事件，要关闭连接
            break;
        }
        LOG_INFO("[新用户上线~]");
    } while (1);
    
    return;
}

void CSocket::ReadRequestHandler(ConnectionItem* _Conn)
{
    LOG_INFO("来消息了");
}

void CSocket::WriteRequestHandler(ConnectionItem* _Conn)
{
    LOG_INFO("可以发消息了");
}