#pragma once
#include "command.h"
#include "icommandlinereader.h"
#include "nameformat.h"
#include "validator.h"
#include "initializedoptional.h"
#include <gsl/assert>

namespace cmdlime{
class Config;
}

namespace cmdlime::detail{

template<typename TCfg>
class CommandCreator{
    static_assert(std::is_base_of_v<Config, TCfg>,
                  "TCfg must be a subclass of cmdlime::Config.");
public:
    CommandCreator(CommandLineReaderPtr reader,
                   const std::string& varName,
                   InitializedOptional<TCfg>& commandValue,
                   typename Command<TCfg>::Type type = Command<TCfg>::Type::Normal)
            : reader_(reader)
            , commandValue_(commandValue)
    {
        Expects(!varName.empty());
        nestedReader_ = reader_ ? reader_->makeNestedReader(NameFormat::fullName(reader_->format(), varName)) : CommandLineReaderPtr{};
        command_ = std::make_unique<Command<TCfg>>(
                reader_ ? NameFormat::fullName(reader->format(), varName) : varName,
                commandValue,
                nestedReader_,
                type);
    }

    auto& operator<<(const std::string& info)
    {
        command_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        command_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(std::function<void(const InitializedOptional<TCfg>&)> validationFunc)
    {
        if (reader_)
            reader_->addValidator(std::make_unique<Validator<InitializedOptional<TCfg>>>(
                    *command_,
                    commandValue_,
                    std::move(validationFunc)));
        return *this;
    }

    operator InitializedOptional<TCfg>()
    {
        if (reader_)
            reader_->addCommand(std::move(command_));
        return InitializedOptional<TCfg>{nestedReader_};
    }

private:
    std::unique_ptr<Command<TCfg>> command_;
    CommandLineReaderPtr reader_;
    CommandLineReaderPtr nestedReader_;
    InitializedOptional<TCfg>& commandValue_;
};

}