#pragma once
#include "ioption.h"
#include "iconfigreader.h"
#include <vector>
#include <string>
#include <memory>

namespace cmdlime::detail{
class OptionInfo;
class IFlag;
class IConfig;

class ICommand : public IOption{
public:
    virtual bool hasValue() const = 0;
    virtual ConfigReaderPtr configReader() const = 0;
    virtual ConfigReadResult read(const std::vector<std::string>& commandLine) = 0;
    virtual bool isSubCommand() const = 0;
    virtual void enableHelpFlag() = 0;
    virtual bool isHelpFlagSet() const = 0;
    virtual std::string usageInfo() const = 0;
    virtual std::string usageInfoDetailed() const = 0;
    virtual void setUsageInfoFormat(const UsageInfoFormat&) = 0;
    virtual void setCommandName(const std::string& parentCommandName) = 0;
    virtual void validate() const = 0;
};

}
