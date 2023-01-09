#ifndef CMDLIME_UTILS_H
#define CMDLIME_UTILS_H

#include "initializedoptional.h"
#include "nameof_import.h"
#include "external/sfun/type_traits.h"
#include <optional>
#include <sstream>
#include <string>
#include <tuple>

namespace cmdlime::detail {

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
    auto check = [&str](auto num)
    {
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
    auto result = [&]
    {
        if constexpr (sfun::is_optional_v<type> || sfun::is_dynamic_sequence_container_v<type>)
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

} //namespace cmdlime::detail

#endif //CMDLIME_UTILS_H
