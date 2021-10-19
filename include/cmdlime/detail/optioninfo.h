#pragma once
#include <gsl/gsl>
#include <string>
#include <sstream>
#include <utility>

namespace cmdlime::detail{

class OptionInfo{
public:
    OptionInfo(std::string name, std::string shortName, std::string valueName)
        : name_(std::move(name))
        , shortName_(std::move(shortName))
        , valueName_(std::move(valueName))
    {
        Expects(!name_.empty());
    }

    void addDescription(const std::string& desc)
    {
        description_ += desc;
    }

    void resetName(const std::string& name)
    {
        Expects(!name.empty());
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
        return description_;
    }

private:
    std::string name_;
    std::string shortName_;
    std::string valueName_;
    std::string description_;
};

}
