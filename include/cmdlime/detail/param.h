#pragma once
#include "iparam.h"
#include "optioninfo.h"
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <cmdlime/stringconverter.h>
#include <sstream>
#include <optional>
#include <memory>
#include <functional>

namespace cmdlime::detail{

template<typename T>
class Param : public IParam{
public:
    Param(std::string name,
          std::string shortName,
          std::string type,
          T& paramValue)
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
    bool read(const std::string& data) override
    {
        auto paramValue = convertFromString<T>(data);
        if (!paramValue)
            return false;

        paramValue_ = *paramValue;
        hasValue_ = true;
        return true;
    }

    bool hasValue() const override
    {
        return hasValue_;
    }

    bool isOptional() const override
    {
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

}
