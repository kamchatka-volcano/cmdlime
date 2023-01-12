#ifndef CMDLIME_STRINGCONVERTER_H
#define CMDLIME_STRINGCONVERTER_H

#include "errors.h"
#include "detail/external/sfun/type_traits.h"
#include "detail/utils.h"
#include <optional>
#include <sstream>
#include <string>
#include <variant>

namespace cmdlime {

template<typename T>
struct StringConverter {
    static std::optional<std::string> toString(const T& value)
    {
        if constexpr (sfun::is_optional_v<T>) {
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
        [[maybe_unused]] auto setValue = [](auto& value, const std::string& data) -> std::optional<T>
        {
            auto stream = std::stringstream{data};
            stream >> value;

            if (stream.bad() || stream.fail() || !stream.eof())
                return {};
            return value;
        };

        if constexpr (std::is_convertible_v<sfun::remove_optional_t<T>, std::string>) {
            return data;
        }
        else if constexpr (sfun::is_optional_v<T>) {
            auto value = T{};
            value.emplace();
            return setValue(*value, data);
        }
        else {
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
    catch (...) {
        return {};
    }
}

template<typename T>
T convertFromString(const std::string& data)
{
    try {
        const auto result = StringConverter<T>::fromString(data);
        if (!result)
            throw StringConversionError{};
        return result.value();
    }
    catch (const ValidationError& error) {
        throw StringConversionError{error.what()};
    }
    catch (...) {
        throw StringConversionError{};
    }
}

} //namespace detail
} //namespace cmdlime

#endif //CMDLIME_STRINGCONVERTER_H