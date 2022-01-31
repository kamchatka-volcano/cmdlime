#pragma once
#include "detail/utils.h"
#include <string>
#include <sstream>
#include <optional>


namespace cmdlime{

template<typename T>
struct StringConverter{
    static std::optional<std::string> toString(const T& value)
    {
        if constexpr(detail::is_optional<T>::value){
            if (!value)
                return {};
            auto stream = std::stringstream{};
            stream << *value;
            return stream.str();
        }
        else {
            auto stream = std::stringstream{};
            stream << value;
            return stream.str();
        }
    }

    static std::optional<T> fromString(const std::string& data)
    {
        auto setValue = [](auto& value, const std::string& data) -> std::optional<T>
        {
            auto stream = std::stringstream{data};
            stream >> value;

            if (stream.bad() || stream.fail() || !stream.eof())
                return {};
            return value;
        };

        if constexpr(std::is_convertible_v<T, std::string>){
            return data;
        }
        else if constexpr(detail::is_optional<T>::value){
            auto value = T{};
            value.emplace();
            return setValue(*value, data);
        }
        else{
            auto value = T{};
            return setValue(value, data);
        }
    }
};

namespace detail {

template<typename T>
std::optional<std::string> convertToString(const T& value)
{
    try {
        return StringConverter<T>::toString(value);
    }
    catch(...){
        return {};
    }
}

template<typename T>
std::optional<T> convertFromString(const std::string& data)
{
    try {
        return StringConverter<T>::fromString(data);
    }
    catch(...){
        return {};
    }
}

}
}