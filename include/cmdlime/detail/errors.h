#pragma once
#include <stdexcept>

namespace cmdlime{

class ParsingError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

}
