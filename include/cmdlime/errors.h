#pragma once
#include <stdexcept>

namespace cmdlime{

class Error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class ParsingError : public Error
{
    using Error::Error;
};

class ConfigError : public Error
{
    using Error::Error;
};

class CommandError : public Error
{
public:
    CommandError(const std::string& commandName,
                 const std::string& commandUsageInfo,
                 const std::string& errorMsg)
        : Error(errorMsg)
        , commandName_(commandName)
        , commandUsageInfo_(commandUsageInfo)
    {
    }

    const std::string& commandName() const
    {
        return commandName_;
    }

    const std::string& commandUsageInfo() const
    {
        return commandUsageInfo_;
    }

private:
    std::string commandName_;
    std::string commandUsageInfo_;
};

class CommandParsingError : public CommandError
{
public:
    CommandParsingError(const std::string& commandName,
                        const std::string& commandUsageInfo,
                        const ParsingError& error)
        : CommandError(commandName, commandUsageInfo, error.what())
    {}
};


class CommandConfigError : public CommandError
{
public:
    CommandConfigError(const std::string& commandName,
                       const std::string& commandUsageInfo,
                       const ConfigError& error)
        : CommandError(commandName, commandUsageInfo, error.what())
    {}
};


}
