#ifndef CMDLIME_PARAMLIST_H
#define CMDLIME_PARAMLIST_H

#include "iparamlist.h"
#include "optioninfo.h"
#include "external/sfun/string_utils.h"
#include "external/sfun/type_traits.h"
#include <cmdlime/customnames.h>
#include <cmdlime/errors.h>
#include <cmdlime/stringconverter.h>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>

namespace cmdlime::detail {

template<typename TParamList>
class ParamList : public IParamList {
    static_assert(sfun::is_dynamic_sequence_container_v<TParamList>, "Param list field must be a sequence container");

public:
    ParamList(std::string name, std::string shortName, std::string type, TParamList& paramListValue)
        : info_(std::move(name), std::move(shortName), std::move(type))
        , paramListValue_(paramListValue)
    {
    }

    void setDefaultValue(const TParamList& value)
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
    void read(const std::string& data) override
    {
        if (!isDefaultValueOverwritten_) {
            paramListValue_.clear();
            isDefaultValueOverwritten_ = true;
        }

        const auto dataParts = sfun::split(data, ",");
        for (const auto& part : dataParts) {
            auto paramVal = convertFromString<typename TParamList::value_type>(std::string{part});
            paramListValue_.emplace_back(std::move(paramVal));
        }
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
    TParamList& paramListValue_;
    bool hasValue_ = false;
    std::optional<TParamList> defaultValue_;
    bool isDefaultValueOverwritten_ = false;
};

} //namespace cmdlime::detail

#endif //CMDLIME_PARAMLIST_H