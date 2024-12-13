#ifndef __STDAFX_H__
#define __STDAFX_H__

/**
 * @desrcibe:通用头文件保存，后续使用的文件可以直接包含此文件即可
 */


#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/un.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <climits>
#include <semaphore.h>

#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <atomic>
#include <list>
#include <memory>       //智能指针需要用到的库


#endif