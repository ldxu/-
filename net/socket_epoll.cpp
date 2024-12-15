#include "../include/socket.h"

int CSocket::EpollInit()
{
    // step1 创建epoll实例
    c_epollFd = epoll_create(EPOLL_EVENTS_SIZE);
    if (c_epollFd == -1)
    {
        LOG_ERROR("EpollInit() 中 epoll_create() failed. error:{%s}", strerror(errno));
        exit(2);//最核心的epoll都启动失败了，直接退出即可
    }
    // step2 启动连接池
    IninConnections();

    // step3 遍历所有的监听端口，给每个监听端口分配一个连接
    for (auto listenItem:c_listenSocketVec)
    {
        auto _Conn = GetConnection(listenItem->c_fd);
        if (_Conn == nullptr)
        {
            LOG_ERROR("连接池异常, EpollInit() 中 GetConnection() failed().");
            exit(2);    //开句连接池就挂了，那程序也可以直接挂了
        }

        _Conn->c_listen = listenItem;
        listenItem->c_connect = _Conn;          // 连接对象和监听对象互相连接，记得使用weak_ptr防止出现循环引用问题
        _Conn->c_readHandler = &CSocket::AcceptHandler;
        // 在这里是要绑定一个handler的，即epoll事件的处理函数，但是观察一下是否还有其他机会可以放

        if (EpollOperEvent(
            _Conn->c_fd,
            EPOLL_CTL_ADD,
            EPOLLIN | EPOLLRDHUP | EPOLLET,
            0,
            _Conn
            ) == -1)
        {
            exit(2);    //添加epoll实例出现问题，直接退出了
        }
    }
    return 1;

}

int CSocket::EpollOperEvent(int fd,                                         //监听的文件句柄
                            uint32_t eventType,                             //EPOLL_CTL_ADD、EPOLL_CTL_MOD、EPOLL_CTL_DEL
                            uint32_t flag,                                  //标志，监听的具体事件类型
                            int baction,                                    //补充动作，在MOD下有效
                            std::shared_ptr<ConnectionItem> _Conn           //一个连接，为了方便在处理时间的时候找到
                )
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    if (eventType == EPOLL_CTL_ADD)
    {
        ev.events = flag;
        _Conn->c_eventType = flag;
    }
    else if( eventType == EPOLL_CTL_MOD )       //可以理解为，在原有的基础上进行事件修改，所以需要一些额外的字段来辅助判断
    {
        ev.events = _Conn->c_eventType; //从连接中回复原来的epoll事件状态
        if (baction == 0)
        {
            // 增加一个标记
            ev.events |= flag;
        }
        else if (baction == 1)
        {
            // 去掉某个标记
            ev.events &= ~flag;
        }
        else
        {
            ev.events = flag;
        }
        _Conn->c_eventType = ev.events;
    }
    else
    {
        // 删除红黑树中的节点，socket关闭了这个会自动关闭，不需要特意修改
        return 1;
    }

    // 把智能指针的地址传进去，然后通过这个指针把智能指针转出来
    ev.data.ptr = (void*)_Conn.get();
    if (epoll_ctl(c_epollFd, eventType, fd, &ev) == -1)
    {
        LOG_ERROR("EpollOperEvent() 中 epoll_ctl failed. error:{%s}", std::strerror(errno));
        return -1;
    }
    return 1;
}
/// @brief 处理Epoll的事件通知
/// @param timer 
/// @return 1 正常返回 0 异常返回
int CSocket::EpollProcessEvent(int timer)
{
    int events = epoll_wait(c_epollFd, c_events, EPOLL_EVENTS_SIZE, timer); //使用epoll_wait得到感兴趣的事件

    if (events < 0)
    {
        if (errno == EINTR)
        {
            LOG_WARN("EpollProcessEvent 中 发生系统调用被中断事件，需要注意!");
            return 1;
        }
        else
        {
            LOG_ERROR("EpollProcessEvent 中 发生错误! error:{%s}", std::strerror(errno));
            return 0;
        }
    }
    if (events == 0)
    {
        if (timer != -1)
        {
            return 1;
        }
        LOG_INFO("没有事件发生!");
        return 0;
    }

    uint32_t    revents;
    for (int i=0;i < events;++i)
    {
        // 进行指针的类型转换，得到对应的连接指针
        auto _Conn = static_cast<ConnectionItem*>(c_events[i].data.ptr);
        revents = c_events[i].events;

        if (revents & EPOLLIN) // 如果是读事件
        {
            (this->*(_Conn->c_readHandler))(_Conn); //从指针中得到函数指针地址，然后运行那个函数
        }

        if (revents & EPOLLOUT)
        {
            (this->*(_Conn->c_writeHandler))(_Conn); //从指针中得到函数指针地址，然后运行那个函数
        }
    }
    return 1;
}

