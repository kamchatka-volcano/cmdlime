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

}
