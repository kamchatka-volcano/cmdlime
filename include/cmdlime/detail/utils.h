#pragma once
#include "initializedoptional.h"
#include "nameof_import.h"
#include <string>
#include <sstream>
#include <optional>
#include <gsl/assert>

namespace cmdlime::detail{

[[noreturn]]
inline void ensureNotReachable()
{
    Ensures(false);
}

inline std::string capitalize(const std::string& input)
{
    if (input.empty())
        return {};
    auto result = input;
    result[0] = static_cast<char>(std::toupper(static_cast<int>(result[0])));
    return result;
}

inline bool isNumber(const std::string& str)
{
    auto check = [&str](auto num){
        std::stringstream stream{str};
        stream >> num;
        return !stream.bad() && !stream.fail() && stream.eof();
    };
    return check(int64_t{}) || check(double{});
}


#ifdef CMDLIME_NAMEOF_AVAILABLE
template<typename TCfg>
inline std::string nameOfType()
{
    using type = std::remove_const_t<std::remove_reference_t<TCfg>>;
    auto result = [&]{
        if constexpr(is_optional<type>::value)
            return std::string{nameof::nameof_short_type<typename type::value_type>()};
        else
            return std::string{nameof::nameof_short_type<type>()};
    }();
    if (result == "basic_string")
        result = "string";

    return result;
}

template<auto member, typename TParent>
inline std::tuple<std::string, std::string> getMemberPtrNameAndType(TParent* parentPtr)
{
    const auto memberName = std::string{nameof::nameof_member<member>()};
    const auto& memberValue = parentPtr->*member;
    return std::make_tuple(memberName, nameOfType<decltype(memberValue)>());
}
#endif

template<typename T, typename = void>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
inline constexpr auto is_optional_v = is_optional<T>::value;

template<typename T, typename = void>
struct remove_optional{
    using type = T;
};

template<typename T>
struct remove_optional<std::optional<T>>{
    using type = T;
};

template <typename T>
using remove_optional_t = typename remove_optional<T>::type;

}
