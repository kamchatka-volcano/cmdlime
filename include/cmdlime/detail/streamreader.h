#pragma once
#include <sstream>
#include <optional>

namespace cmdlime::detail{

template<typename T>
std::stringstream& operator >>(std::stringstream& stream, std::optional<T>& val)
{
    auto value = T{};
    stream >> value;
    val = value;
    return stream;
}

template<typename T>
std::stringstream& operator <<(std::stringstream& stream, const std::optional<T>& val)
{
    if (val)
        stream << val.value();
    return stream;
}

template <typename T>
bool readFromStream(std::stringstream& stream, T& value)
{
    try{
        stream >> value;
    }
    catch(const std::exception&){
        return false;
    }
    if (stream.bad() || stream.fail() || !stream.eof())
        return false;
    return true;
}

}
