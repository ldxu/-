
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "stdafx.h"
#include "common.h"
#include "config.h"	//读取配置文件
#include "macro.h"
#include "../log/log.h"
using HEADER = COMM_PKG_HEADER;
struct ConnectionItem;
typedef class CSocket CSocket;
typedef void (CSocket::*EventHandler)(ConnectionItem* _Conn);	//定义成员函数指针
enum class ConnectState
{
	WAIT_HEADER,								//准备接收包头
	WAIT_BODY,									//准备接收包体
	PROCESSING									//处理数据包						
};//收包的几种状态
// 监听结构体
struct ListenItem
{
	int 								c_port;			//连接的端口号
	int 								c_fd;			//连接端口的套接字
	std::shared_ptr<ConnectionItem>		c_connect;
	ListenItem(int _port, int _fd):c_port(_port), c_fd(_fd){}
	~ListenItem()
	{
		if (c_fd != -1)
		{
			close(c_fd);
			c_fd = -1;
		}
	}
};

// 连接结构体
struct ConnectionItem
{

	ConnectionItem();
	virtual ~ConnectionItem();

	void GetOneToUse();					//分配一个连接
	void PutOneToFree();				//释放一个连接的资源

	// 思考每个连接需要的一些关键信息  首先是套接字文件描述符fd, 它从哪个监听端口得到的需要记录
	int									c_fd;				//socket
	std::weak_ptr<ListenItem>			c_listen;			//指向监听套接字
	uint64_t							c_sequence;			//记录每个连接的序号，递增的
	struct sockaddr 					c_clientSock;		//保留客户端的相关信息

	EventHandler						c_readHandler;		//读操作执行的操作
	EventHandler						c_writeHandler;		//写操作执行的操作
	/*
		在知道包头的情况下，因为包头里面有保存数据包的大小，那么我们只需要一个额外的
		offset来记录包体的写入偏移就行，然后就是一个接收缓冲区，对于发送，那就是一股脑发
		用一个offset来记录发到哪里就行了
	*/
	ConnectState						c_statu;			//记录当前连接的状态
	HEADER								c_header;			//接受包头数据
	unsigned int 						c_recvOffset;		//接收缓冲区偏移
	// 后续使用内存池来解决这些问题，现在就使用纯粹的C数组来做缓冲区
	char*								c_recvBuffer;		//接受数据缓冲区

	unsigned int						c_sendOffset;		//发送数据的偏移
	char*								c_sendBuffer;		//发送数据缓冲区
	
	uint32_t							c_eventType;		//连接也保存epoll事件标志
	std::atomic<int>					c_sendCount;		//发送队列的大小，如果只收不发，可以根据这个判断踢出
};
//------------------------------------
//socket相关类
class CSocket
{
public:
	CSocket();                                                            //构造函数
	virtual ~CSocket();                                                   //释放函数
    virtual bool Initialize();
	virtual	bool SpawnSysThread(){}											//开启一些专用线程

	void IninConnections();
	std::shared_ptr<ConnectionItem> GetConnection(int fd);				
	void CloseConnection(shared_ptr<ConnectionItem>& _Conn);
	void FreeConnection(shared_ptr<ConnectionItem>& _Conn);
	// epoll 相关 ------------------------------------------------------------

	static int CloseFileDescriptor(int& fd);
	
public:
	int EpollInit();														//初始化epoll，得到epoll实例
	int EpollOperEvent(int fd, uint32_t eventType, uint32_t flag, int baction, std::shared_ptr<ConnectionItem> _Conn);	//添加监听的事件
	int EpollProcessEvent(int timer);										//调用这个函数来处理来到的epoll事件
	void AcceptHandler(ConnectionItem* _Conn);				//监听套接字的读事件处理函数
	void ReadRequestHandler(ConnectionItem* _Conn);			//用户连接的读请求处理函数
	void WriteRequestHandler(ConnectionItem* _Conn);		//用户连接写请求处理函数 												
private:
	int												c_epollFd;				//epoll对象id
	struct epoll_event								c_events[EPOLL_EVENTS_SIZE];	//接收epoll事件返回的数组
	// END -------------------------------------------------------------------
private:
	bool ReadConfig();
	bool OpenListeningSocket();
	bool SetNonBlocking(int sockfd);
	//使用一个vector容器来保存监听套接字相关的数据
	std::vector<std::shared_ptr<ListenItem>> 		c_listenSocketVec;		//使用智能指针管理
	std::list<std::shared_ptr<ConnectionItem>>		c_connectionList;		//连接池的总大小
	std::list<std::shared_ptr<ConnectionItem>>		c_freeConnectionlist;	//空闲连接池队列
	int												c_workerConnections;	//连接池的大小
	int												c_listenSockCount;
	std::atomic<int>								c_freeWorkerConnections;//空闲可用连接大小
	std::atomic<int>								c_totalConnections;		//连接池总大小
	std::mutex										c_freeMutex;			//和空闲连接队列相关的锁


	std::atomic<int>               					c_onlineUsers;			//在线用户数量


};

#endif
