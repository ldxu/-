#ifndef _CONFIG_H__
#define _CONFIG_H__

#include "stdafx.h"

class CConfig
{
    private:
        CConfig() = default;
    public:
        ~CConfig() = default;
        // 禁止拷贝构造
        CConfig(const CConfig&) = delete;

        // 禁止赋值运算符
        CConfig& operator=(const CConfig&) = delete;
        // 禁用移动构造
        CConfig(CConfig&&) = delete;
        
        //禁用移动赋值
        CConfig& operator=(CConfig&&) = delete;

    public:
        //单例模式
        static CConfig& Instance()
        {
            static CConfig instance;
            return instance;
        }
    public:
        bool Load(const std::string& configName);
        std::string GetString(const std::string& elementName);
        int GetInt(const std::string& elementName, int def);
        // 用于保存配置项
        std::unordered_map<std::string, std::string> m_configMap;
    
};

#endif