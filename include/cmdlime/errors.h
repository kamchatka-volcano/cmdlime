#pragma once
#include <stdexcept>
#include <utility>

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
    CommandError(std::string commandName,
                 std::string commandUsageInfo,
                 const std::string& errorMsg)
        : Error(errorMsg)
        , commandName_(std::move(commandName))
        , commandUsageInfo_(std::move(commandUsageInfo))
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
    CommandParsingError(std::string commandName,
                        std::string commandUsageInfo,
                        const ParsingError& error)
        : CommandError(std::move(commandName), std::move(commandUsageInfo), error.what())
    {}
};


class CommandConfigError : public CommandError
{
public:
    CommandConfigError(std::string commandName,
                       std::string commandUsageInfo,
                       const ConfigError& error)
        : CommandError(std::move(commandName), std::move(commandUsageInfo), error.what())
    {}
};


}
