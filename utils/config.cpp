#include "../include/config.h"
#include "tools.h"
    bool CConfig::Load(const std::string& configName)
    {
            std::ifstream fp(configName);
            if (!fp.is_open())
                {
                    std::cout << "failed to open configuration file :" << configName << std::endl;
                    return false;
                }
            std::string line;
            while (std::getline(fp, line))
            {
                // 去除收尾空格
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n")+1);

                if (line.empty() || line[0] == ' ' || line[0] == '#' || line[0] == '[')
                    continue;
                size_t pos = line.find('=');
                if (pos != std::string::npos)
                {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);

                    // 去除 key 和 value 的首尾空白
                    key.erase(0, key.find_first_not_of(" \t\r\n"));
                    key.erase(key.find_last_not_of(" \t\r\n") + 1);
                    value.erase(0, value.find_first_not_of(" \t\r\n"));
                    value.erase(value.find_last_not_of(" \t\r\n") + 1);

                    // 不允许为空
                    if (!key.empty() && !value.empty())
                        m_configMap[key] = value;
                }
            }

        
        fp.close();
        return true;

    }
    std::string CConfig::GetString(const std::string& elementName) 
    {
        if (m_configMap.count(elementName))
        {
            return m_configMap[elementName];
        }
        else
        {
            return   "Invalid";

        }
    }
    int CConfig::GetInt(const std::string& elementName, int def)
    {
        if (m_configMap.count(elementName))
        {
            try 
            {
                int num = std::stoi(m_configMap[elementName]);
                return num;
            } 
            catch (const std::invalid_argument& e) 
            {
                std::cout<<"invalid argument"<<std::endl;
                return INT_MAX;
            }
        }
        else
        {
            std::cout<<"no element name"<<std::endl;
            return INT_MAX;
        }
    }
