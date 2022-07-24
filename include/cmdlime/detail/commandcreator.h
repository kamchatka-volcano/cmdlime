#pragma once
#include "command.h"
#include "iconfigreader.h"
#include "nameformat.h"
#include "validator.h"
#include "gsl_assert.h"
#include "initializedoptional.h"

namespace cmdlime{
class Config;
}

namespace cmdlime::detail{

template<typename TCfg>
class CommandCreator{
    static_assert(std::is_base_of_v<Config, TCfg>,
                  "TCfg must be a subclass of figcone::Config.");
public:
    CommandCreator(ConfigReaderPtr cfgReader,
                   const std::string& varName,
                   InitializedOptional<TCfg>& commandValue,
                   typename Command<TCfg>::Type type = Command<TCfg>::Type::Normal)
            : cfgReader_(cfgReader)
            , commandValue_(commandValue)
    {
        Expects(!varName.empty());
        nestedCfgReader_ = cfgReader_ ? cfgReader_->makeNestedReader(NameFormat::fullName(cfgReader_->format(), varName)) : ConfigReaderPtr{};
        command_ = std::make_unique<Command<TCfg>>(
                cfgReader_ ? NameFormat::fullName(cfgReader->format(), varName) : varName,
                commandValue,
                nestedCfgReader_,
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
        if (cfgReader_)
            cfgReader_->addValidator(std::make_unique<Validator<InitializedOptional<TCfg>>>(
                    *command_,
                    commandValue_,
                    std::move(validationFunc)));
        return *this;
    }

    operator InitializedOptional<TCfg>()
    {
        if (cfgReader_)
            cfgReader_->addCommand(std::move(command_));
        return InitializedOptional<TCfg>{nestedCfgReader_};
    }

private:
    std::unique_ptr<Command<TCfg>> command_;
    ConfigReaderPtr cfgReader_;
    ConfigReaderPtr nestedCfgReader_;
    InitializedOptional<TCfg>& commandValue_;
};

}