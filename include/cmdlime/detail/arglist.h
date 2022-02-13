#pragma once
#include "iarglist.h"
#include "optioninfo.h"
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <cmdlime/stringconverter.h>
#include <vector>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class ArgList : public IArgList{
public:
    ArgList(std::string name,
            std::string type,
            std::vector<T>& argListValue)
        : info_(std::move(name), {}, std::move(type))
        , argListValue_(argListValue)
    {
    }

    void setDefaultValue(const std::vector<T>& value)
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
        return OptionType::ArgList;
    }

private:
    bool read(const std::string& data) override
    {
        if (!isDefaultValueOverwritten_){
            argListValue_.clear();
            isDefaultValueOverwritten_ = true;
        }
        auto argVal = convertFromString<T>(data);
        if (!argVal)
            return false;
        argListValue_.emplace_back(*argVal);
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
        auto stream = std::stringstream{};
        stream << "{";
        auto firstVal = true;
        for (auto& val : *defaultValue_){
            if (firstVal)
                stream << ", ";
            firstVal = false;
            auto valStr = convertToString(val);
            if (!valStr)
                return {};
            stream << *valStr;
        }
        stream << "}";
        return stream.str();
    }

private:
    OptionInfo info_;
    std::vector<T>& argListValue_;
    bool hasValue_ = false;
    std::optional<std::vector<T>> defaultValue_;
    bool isDefaultValueOverwritten_ = false;
};

}
