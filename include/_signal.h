/**
 * @description:信号处理相关的函数库
 */

#ifndef __SIGNAL_H__
#define __SIGNAL_H__
#include "stdafx.h"



void BlockSignals() {
    sigset_t newMask;
    sigemptyset(&newMask);  // 清空信号集
    sigaddset(&newMask, SIGCHLD);  // 屏蔽子进程退出信号
    sigaddset(&newMask, SIGHUP);   // 屏蔽挂断信号
    sigaddset(&newMask, SIGTERM);  // 屏蔽终止信号
    sigprocmask(SIG_BLOCK, &newMask, nullptr);  // 应用信号屏蔽
}

// 恢复信号
void UnBlockSignals() {
    sigset_t oldMask;
    sigemptyset(&oldMask);  // 清空信号集，相当于解除屏蔽
    sigprocmask(SIG_SETMASK, &oldMask, nullptr);
}

#endif