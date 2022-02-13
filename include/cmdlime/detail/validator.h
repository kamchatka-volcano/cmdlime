#pragma once
#include "ivalidator.h"
#include "ioption.h"
#include "optioninfo.h"
#include "utils.h"
#include <cmdlime/errors.h>
#include <functional>

namespace cmdlime::detail {

inline std::string validatorOptionTypeName(OptionType optionType)
{
    switch(optionType){
        case OptionType::Arg: return "argument";
        case OptionType::ArgList: return "argument list";
        case OptionType::Command: return "command";
        case OptionType::Subcommand: return "subcommand";
        case OptionType::Flag: return "flag";
        case OptionType::ExitFlag: return "exit flag";
        case OptionType::Param: return "parameter";
        case OptionType::ParamList: return "parameter list";
        default:
            using optionTypeIsHandled = std::false_type;
            Ensures(optionTypeIsHandled{});
    }
}

template<typename T>
class Validator : public IValidator
{
public:
    Validator(IOption& option, T& optionValue, std::function<void(const T&)> validatingFunc)
        : option_(option)
        , optionValue_(optionValue)
        , validatingFunc_(std::move(validatingFunc))
    {}

private:
    void validate(const std::string& commandName) const override
    {
        auto makeErrorMessage = [&](const auto& message){
            auto prefix = commandName.empty() ?
                capitalize(validatorOptionTypeName(option_.type())) :
                "Command '" + commandName + "'s " + validatorOptionTypeName(option_.type());
            return prefix + " '" + option_.info().name() + "' is invalid: " + message;
        };

        try{
            validatingFunc_(optionValue_);
        }
        catch(const ValidationError& e){
            throw ParsingError{makeErrorMessage(e.what())};
        }
        catch(...){
            throw ParsingError{makeErrorMessage("Unexpected error")};
        }
    }

    OptionType optionType() const override
    {
        return option_.type();
    }

    IOption& option_;
    T& optionValue_;
    std::function<void(const T&)> validatingFunc_;
};

}