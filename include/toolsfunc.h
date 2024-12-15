#ifndef __TOOLSFUNC_H__
#define __TOOLSFUNC_H__
#include "stdafx.h"
// 字符串相关函数
std::string& Ltrim(std::string& str, const char cc );
std::string& Rtrim(std::string& str, const char cc );
std::vector<std::string>& Split(const std::string& str, std::vector<std::string>& dest, const std::string& delimiter);
std::vector<std::string> Split(const std::string& str, const std::string& delimiter);

//设置可执行程序标题相关函数
void SetProcessName(const char* title, int argc, char* argv[]);

//信号、进程相关函数
int Daemon(pid_t& m_pid);

//信号相关
int InitSignals();

// epoll 相关
void MainProcessCycle();
// epoll循环事件
void ProcessEventsLoop();
#endif