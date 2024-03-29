#ifndef CMDLIME_ICOMMANDLINEREADER_H
#define CMDLIME_ICOMMANDLINEREADER_H

#include "commandlinereaderptr.h"
#include "external/sfun/interface.h"
#include <cmdlime/format.h>
#include <memory>
#include <string>
#include <vector>

namespace cmdlime {
struct UsageInfoFormat;
}

namespace cmdlime::detail {
class Options;
class IParam;
class IParamList;
class IFlag;
class IArg;
class IArgList;
class ICommand;
class IValidator;

enum CommandLineReadResult {
    Completed,
    StoppedOnExitFlag
};

class ICommandLineReader : private sfun::interface<ICommandLineReader> {
public:
    virtual CommandLineReadResult read(const std::vector<std::string>& cmdLine) = 0;
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
    virtual bool shortNamesEnabled() const = 0;
    virtual CommandLineReaderPtr makeNestedReader(const std::string& name) = 0;

protected:
    CommandLineReaderPtr makePtr()
    {
        return this;
    }

    template<typename TCfg>
    void resetCommandLineReader(TCfg& cfg)
    {
        cfg.reader_ = CommandLineReaderPtr{};
    }
};

} //namespace cmdlime::detail

#endif //CMDLIME_ICOMMANDLINEREADER_H