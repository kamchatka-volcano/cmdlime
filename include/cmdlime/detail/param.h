#ifndef CMDLIME_PARAM_H
#define CMDLIME_PARAM_H

#include "iparam.h"
#include "optioninfo.h"
#include "external/sfun/type_traits.h"
#include <cmdlime/customnames.h>
#include <cmdlime/errors.h>
#include <cmdlime/stringconverter.h>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>

namespace cmdlime::detail {

template<typename T>
class Param : public IParam {
public:
    Param(std::string name, std::string shortName, std::string type, T& paramValue)
        : info_(std::move(name), std::move(shortName), std::move(type))
        , paramValue_(paramValue)
    {
    }

    void setDefaultValue(const T& value)
    {
        hasValue_ = true;
        defaultValue_ = value;
    }

    OptionInfo& info() override
    {
        return info_;
    }

    const OptionInfo& info() const override
    {
        return info_;
    }

    OptionType type() const override
    {
        return OptionType::Param;
    }

private:
    void read(const std::string& data) override
    {
        paramValue_ = convertFromString<T>(data);
        hasValue_ = true;
    }

    bool hasValue() const override
    {
        if constexpr (sfun::is_optional_v<T>)
            return true;
        else
            return hasValue_;
    }

    bool isOptional() const override
    {
        if constexpr (sfun::is_optional_v<T>)
            return true;
        else
            return defaultValue_.has_value();
    }

    std::string defaultValue() const override
    {
        if (!defaultValue_)
            return {};
        auto defaultValueStr = convertToString(*defaultValue_);
        if (!defaultValueStr)
            return {};
        if (defaultValueStr->empty())
            return "\"\"";
        return *defaultValueStr;
    }

private:
    OptionInfo info_;
    T& paramValue_;
    std::optional<T> defaultValue_;
    bool hasValue_ = false;
};

} //namespace cmdlime::detail

#endif //CMDLIME_PARAM_H