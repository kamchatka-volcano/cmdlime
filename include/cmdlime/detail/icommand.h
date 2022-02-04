#pragma once
#include <gsl/pointers>
#include <vector>
#include <string>
#include <memory>

namespace cmdlime::detail{
class OptionInfo;
class IFlag;
class IConfig;

class ICommand{
public:
    virtual ~ICommand() = default;
    virtual OptionInfo& info() = 0;
    virtual const OptionInfo& info() const = 0;
    virtual const IConfig* config() const = 0;
    virtual void read(const std::vector<std::string>& commandLine) = 0;    
    virtual bool isSubCommand() const = 0;
    virtual void enableHelpFlag(const std::string& commandName) = 0;
    virtual bool isHelpFlagSet() const = 0;
    virtual std::string usageInfo() const = 0;
    virtual std::string usageInfoDetailed() const = 0;
};

}
