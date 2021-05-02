#pragma once
#include <string>
#include <sstream>

namespace cmdlime::detail{

inline std::string typeNameWithoutNamespace(const std::string& type)
{
    auto pos = type.rfind(':');
    if (pos == std::string::npos || pos == type.size() - 1)
        return type;
    return std::string(type.begin() + static_cast<int>(pos + 1), type.end());
}

class ConfigVar{
public:
    ConfigVar(const std::string& name, const std::string& shortName, const std::string& type)
        : name_(name)
        , shortName_(shortName)
        , type_(typeNameWithoutNamespace(type))
    {
    }

    void addDescription(const std::string& info)
    {
        info_ += info;
    }

    void resetName(const std::string& name)
    {
        name_ = name;
    }

    void resetShortName(const std::string& shortName)
    {
        shortName_ = shortName;
    }

    const std::string& name() const
    {
        return name_;
    }

    const std::string& shortName() const
    {
        return shortName_;
    }

    const std::string& type() const
    {
        return type_;
    }

    const std::string& description() const
    {
        return info_;
    }

private:
    std::string name_;
    std::string shortName_;
    std::string type_;
    std::string info_;        
};

}
