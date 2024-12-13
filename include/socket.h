
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "stdafx.h"
#include "common.h"
#include "../include/config.h"	//读取配置文件
struct ListenItem
{
	int port;			//连接的端口号
	int fd;				//连接端口的套接字
	ListenItem(int _port, int _fd):port(_port), fd(_fd){}
	~ListenItem()
	{
		if (fd != -1)
		{
			close(fd);
			fd = -1;
		}
	}
};

//------------------------------------
//socket相关类
class CSocket
{
public:
	CSocket();                                                            //构造函数
	virtual ~CSocket();                                                   //释放函数
    virtual bool Initialize();
private:
	bool ReadConfig();
	bool OpenListeningSocket();
	bool SetNonBlocking(int sockfd);
	//使用一个vector容器来保存监听套接字相关的数据
	int						c_listenSockCount;
	std::vector<std::shared_ptr<ListenItem>> c_listenSocketVec;		//使用智能指针管理
};

#endif
