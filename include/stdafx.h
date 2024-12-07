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
#include <unordered_map>
#include <iostream>
#include <vector>
#include <fstream>
#include <climits>
#include <string>

#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>


#endif