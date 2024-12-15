#ifndef __CTASK__
#define __CTASK__
#include"stdafx.h"
class CTask
{
    public:
        CTask()     =   default;
        virtual ~CTask(){}
        virtual void Exec() = 0;    // 申明为纯虚函数
};

class CGeneralTask : public CTask
{
    std::function<void()> _func;
    public:
        CGeneralTask(const std::function<void()>& func)
            :_func(func)
            {

            }
        CGeneralTask(std::function<void()>&& func)
            :_func(func)
            {

            }
        virtual ~CGeneralTask(){}
        virtual void Exec() override { if(_func) _func();}
        
};

#endif 