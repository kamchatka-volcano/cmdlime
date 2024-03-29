#ifndef CMDLIME_PARAMLISTCREATOR_H
#define CMDLIME_PARAMLISTCREATOR_H

#include "icommandlinereader.h"
#include "nameformat.h"
#include "paramlist.h"
#include "validator.h"
#include "external/sfun/precondition.h"
#include "external/sfun/type_traits.h"

namespace cmdlime::detail {

template<typename TParamList>
class ParamListCreator {
    static_assert(sfun::is_dynamic_sequence_container_v<TParamList>, "Param list field must be a sequence container");

public:
    ParamListCreator(
            CommandLineReaderPtr reader,
            sfun::not_empty<const std::string&> varName,
            sfun::not_empty<const std::string&> type,
            TParamList& paramListValue)
        : reader_(reader)
        , paramListValue_(paramListValue)
    {
        paramList_ = std::make_unique<ParamList<TParamList>>(
                reader_ ? NameFormat::name(reader_->format(), varName.get()) : varName.get(),
                reader_ ? NameFormat::shortName(reader_->format(), varName.get()) : varName.get(),
                reader_ ? NameFormat::valueName(reader_->format(), type.get()) : type.get(),
                paramListValue);
    }

    auto& operator<<(const std::string& info)
    {
        paramList_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        paramList_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(const ShortName& customName)
    {
        if (reader_ && reader_->shortNamesEnabled())
            paramList_->info().resetShortName(customName.value());
        return *this;
    }

    auto& operator<<(const WithoutShortName&)
    {
        if (reader_ && reader_->shortNamesEnabled())
            paramList_->info().resetShortName({});
        return *this;
    }

    auto& operator<<(const ValueName& valueName)
    {
        paramList_->info().resetValueName(valueName.value());
        return *this;
    }

    auto& operator<<(std::function<void(const TParamList&)> validationFunc)
    {
        if (reader_)
            reader_->addValidator(
                    std::make_unique<Validator<TParamList>>(*paramList_, paramListValue_, std::move(validationFunc)));
        return *this;
    }

    auto& operator()(TParamList defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        paramList_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator TParamList()
    {
        if (reader_)
            reader_->addParamList(std::move(paramList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ParamList<TParamList>> paramList_;
    TParamList defaultValue_;
    CommandLineReaderPtr reader_;
    TParamList& paramListValue_;
};

} //namespace cmdlime::detail

#endif //CMDLIME_PARAMLISTCREATOR_H