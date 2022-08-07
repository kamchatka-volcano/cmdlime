#pragma once
#include "configreaderptr.h"
#include <vector>
#include <string>
#include <memory>

namespace cmdlime{
struct UsageInfoFormat;
};

namespace cmdlime::detail{
class Options;
class IParam;
class IParamList;
class IFlag;
class IArg;
class IArgList;
class ICommand;
class IValidator;

enum ConfigReadResult{
    Completed,
    StoppedOnExitFlag
};

class IConfigReader{
public:
    IConfigReader() = default;
    virtual ~IConfigReader() = default;
    IConfigReader(const IConfigReader&) = delete;
    IConfigReader& operator=(const IConfigReader&) = delete;
    IConfigReader(IConfigReader&&) = delete;
    IConfigReader& operator=(IConfigReader&&) = delete;

    virtual ConfigReadResult read(const std::vector<std::string>& cmdLine) = 0;
    virtual const std::string& versionInfo() const = 0;
    virtual std::string usageInfo() const = 0;
    virtual std::string usageInfoDetailed() const = 0;
    virtual void setUsageInfoFormat(const UsageInfoFormat&) = 0;
    virtual void setCommandName(const std::string&) = 0;
    virtual void addParam(std::unique_ptr<IParam> param) = 0;
    virtual void addParamList(std::unique_ptr<IParamList> paramList) = 0;
    virtual void addFlag(std::unique_ptr<IFlag> flag) = 0;
    virtual void addArg(std::unique_ptr<IArg> arg) = 0;
    virtual void setArgList(std::unique_ptr<IArgList> argList) = 0;
    virtual void addCommand(std::unique_ptr<ICommand> command) = 0;
    virtual void addValidator(std::unique_ptr<IValidator> validator) = 0;
    virtual void validate(const std::string& commandName) const = 0;
    virtual const Options& options() const = 0;
    virtual Format format() const = 0;
    virtual ConfigReaderPtr makeNestedReader(const std::string& name) = 0;
    virtual void swapContents(ConfigReaderPtr) = 0;

protected:
    ConfigReaderPtr makePtr()
    {
        return this;
    }
};

}