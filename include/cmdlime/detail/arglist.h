#ifndef CMDLIME_ARGLIST_H
#define CMDLIME_ARGLIST_H

#include "iarglist.h"
#include "optioninfo.h"
#include <cmdlime/customnames.h>
#include <cmdlime/errors.h>
#include <cmdlime/stringconverter.h>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>

namespace cmdlime::detail {

template<typename TArgList>
class ArgList : public IArgList {
public:
    ArgList(std::string name, std::string type, TArgList& argListValue)
        : info_(std::move(name), {}, std::move(type))
        , argListValue_(argListValue)
    {
    }

    void setDefaultValue(const TArgList& value)
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
    void read(const std::string& data) override
    {
        if (!isDefaultValueOverwritten_) {
            argListValue_.clear();
            isDefaultValueOverwritten_ = true;
        }
        auto argVal = convertFromString<typename TArgList::value_type>(data);
        argListValue_.emplace_back(std::move(argVal));
        hasValue_ = true;
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
        for (auto& val : *defaultValue_) {
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
    TArgList& argListValue_;
    bool hasValue_ = false;
    std::optional<TArgList> defaultValue_;
    bool isDefaultValueOverwritten_ = false;
};

} //namespace cmdlime::detail

#endif //CMDLIME_ARGLIST_H