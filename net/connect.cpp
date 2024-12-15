#include "../include/socket.h"
/**
 * @brief:  用来写和连接池相关的代码
 * @time:   2024-12-14
 */
void ConnectionItem::PutOneToFree()
{
    ++c_sequence;
    return;
}

void ConnectionItem::GetOneToUse()
{
    c_fd            = -1;                               //把一些初始化的工作做好
    c_statu         = ConnectState::WAIT_HEADER;       //初始化当前连接的状态
    c_recvOffset    = 0;
    c_sendOffset    = 0;
    c_recvBuffer    = nullptr;                          //接收缓冲区先清空
    c_sendBuffer    = nullptr;                          //发送缓冲区也清空
    c_sendCount     = 0;                                //记录发送队列的数量
}

ConnectionItem::ConnectionItem()
{

}

ConnectionItem::~ConnectionItem()
{

}

std::shared_ptr<ConnectionItem> CSocket::GetConnection(int fd)
{
    // 需要访问空闲链表了
    std::lock_guard<std::mutex> lock(c_freeMutex);

    if (c_freeWorkerConnections > 0) //还有空闲连接
    {
        // ->>>>>>>>>> 对共享资源的访问，需要注意相关变量的释放
        auto _Conn = c_freeConnectionlist.front();
        c_freeConnectionlist.pop_front();           //拿出来了就要记得弹出
        --c_freeWorkerConnections;                  //修改可用连接数目
        // -<<<<<<<<<<


        _Conn->GetOneToUse();                       //初始化连接信息
        _Conn->c_fd = fd;                           //标记这个连接是和fd绑定的
        return _Conn;
    }

    // 到了这里那就是没有空闲连接了 可以考虑新加连接，也可以考虑拒绝暂且考虑拒绝
    return nullptr;
}

void CSocket::CloseConnection(shared_ptr<ConnectionItem>& _Conn)
{
    FreeConnection(_Conn);
    if (_Conn->c_fd != -1) // 一般到这一步就关好了，这大概不用执行的有逻辑问题，如果在上一步就放到空闲队列了，如果被取用，文件描述符都没被关闭，那不就出问题了
    {
        close(_Conn->c_fd);
        _Conn->c_fd = -1;
    }
    return;
}

void CSocket::FreeConnection(shared_ptr<ConnectionItem>& _Conn)
{
    // 要操作空闲队列了 要加锁
    std::lock_guard<std::mutex> lock(c_freeMutex);
    // 最关键的要关闭文件描述符
    CloseFileDescriptor(_Conn->c_fd);

    _Conn->PutOneToFree();                  //清空连接中的一些信息
    c_freeConnectionlist.push_back(_Conn);  //放到空闲队列
    ++c_freeWorkerConnections;              //增加可用连接的大小
    return;
}

int CSocket::CloseFileDescriptor(int& fd)
{
    if (fd < 0)
    {
        return -1;
    }

    int result;
    do
    {
        result = close(fd);
    } while (result == -1 && errno == EINTR); // 确保如果是因为系统调用被中断导致的未关闭文件描述符
    if (result == -1)
    {
        switch (errno)
        {
        case EBADF:
            // do nothing
            break;
        case EINTR:
            // do nothing
            break;
        case ENOSYS:
            // do nothing
            break;
        default:
            break;
        }
        return -1;
    }
    fd = -1;
    return 0;
}
