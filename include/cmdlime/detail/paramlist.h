#pragma once
#include "iparamlist.h"
#include "optioninfo.h"
#include "string_utils.h"
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <cmdlime/stringconverter.h>
#include <vector>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{
namespace str = string_utils;

template <typename T>
class ParamList : public IParamList{
public:
    ParamList(std::string name,
              std::string shortName,
              std::string type,
              std::vector<T>& paramListValue)
        : info_(std::move(name), std::move(shortName), std::move(type))
        , paramListValue_(paramListValue)
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
        return OptionType::ParamList;
    }

private:
    bool read(const std::string& data) override
    {
        if (!isDefaultValueOverwritten_){
            paramListValue_.clear();
            isDefaultValueOverwritten_ = true;
        }

        const auto dataParts = str::split(data, ",");
        for (const auto& part : dataParts){
            auto paramVal = convertFromString<T>(part);
            if (!paramVal)
                return false;
            paramListValue_.emplace_back(*paramVal);
        }
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
            auto valStr = convertToString(val);
            if (!valStr)
                return {};

            if (!firstVal)
                stream << ", ";
            firstVal = false;

            if (valStr->empty())
                stream << "\"\"";
            else
                stream << *valStr;
        }
        stream << "}";
        return stream.str();
    }

private:
    OptionInfo info_;
    std::vector<T>& paramListValue_;
    bool hasValue_ = false;
    std::optional<std::vector<T>> defaultValue_;
    bool isDefaultValueOverwritten_ = false;
};

}
