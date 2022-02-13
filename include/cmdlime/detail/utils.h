#pragma once
#include "nameof_import.h"
#include <gsl/pointers>
#include <string>
#include <sstream>
#include <optional>

namespace cmdlime::detail{

template <typename T>
inline std::vector<gsl::not_null<T*>> getPtrList(const std::vector<std::unique_ptr<T>>& ownerList)
{
    auto result = std::vector<gsl::not_null<T*>>{};
    std::transform(ownerList.begin(), ownerList.end(), std::back_inserter(result),
                   [](auto& owner){return owner.get();});
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

template<typename T, typename = void>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T> > : std::true_type {};

#ifdef CMDLIME_NAMEOF_AVAILABLE
template<typename T>
inline std::string nameOfType()
{
    using type = std::remove_const_t<std::remove_reference_t<T>>;
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

}
