#pragma once
#include <string>
#include <sstream>

namespace cmdlime::detail{

class ConfigVar{
public:
    ConfigVar(const std::string& name, const std::string& shortName, const std::string& valueName)
        : name_(name)
        , shortName_(shortName)
        , valueName_(valueName)
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

    void resetValueName(const std::string& name)
    {
        valueName_ = name;
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

    const std::string& valueName() const
    {
        return valueName_;
    }

    const std::string& description() const
    {
        return info_;
    }

private:
    std::string name_;
    std::string shortName_;
    std::string valueName_;
    std::string info_;        
};

}
