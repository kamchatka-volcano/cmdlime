#pragma once
#include "param.h"
#include "paramlist.h"
#include "flag.h"
#include "arg.h"
#include "arglist.h"
#include "command.h"
#include <optional>

#define PARAM(name, type) type name = cmdlime::detail::makeParamCreator<type>(*this, #name, #type, [this]()->type&{return name;})
#define PARAMLIST(name, type) std::vector<type> name = cmdlime::detail::makeParamListCreator<type>(*this, #name, #type, [this]()->std::vector<type>&{return name;})
#define FLAG(name) bool name = cmdlime::detail::FlagCreator{*this, #name, [this]()->bool&{return name;}}
#define EXITFLAG(name) bool name = cmdlime::detail::FlagCreator{*this, #name, [this]()->bool&{return name;}, cmdlime::detail::Flag::Type::Exit}
#define ARG(name, type) type name = cmdlime::detail::makeArgCreator<type>(*this, #name, #type, [this]()->type&{return name;})
#define ARGLIST(name, type) std::vector<type> name = cmdlime::detail::makeArgListCreator<type>(*this, #name, #type, [this]()->std::vector<type>&{return name;})
#define COMMAND(name, type) std::optional<type> name = cmdlime::detail::makeCommandCreator<type>(*this, #name, [this]()->std::optional<type>&{return name;})
#define SUBCOMMAND(name, type) std::optional<type> name = cmdlime::detail::makeCommandCreator<type>(*this, #name, [this]()->std::optional<type>&{return name;}, cmdlime::detail::Command<type>::Type::SubCommand)
