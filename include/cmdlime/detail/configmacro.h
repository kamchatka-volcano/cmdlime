#pragma once
#include "paramcreator.h"
#include "paramlistcreator.h"
#include "flagcreator.h"
#include "argcreator.h"
#include "arglistcreator.h"
#include "commandcreatormaker.h"
#include <vector>
#include <optional>

#define CMDLIME_PARAM(name, type) type name = cmdlime::detail::makeParamCreator<type>(cfgReader(), #name, #type, [this]()->type&{return name;})
#define CMDLIME_PARAMLIST(name, type) std::vector<type> name = cmdlime::detail::makeParamListCreator<type>(cfgReader(), #name, #type, [this]()->std::vector<type>&{return name;})
#define CMDLIME_FLAG(name) bool name = cmdlime::detail::makeFlagCreator(cfgReader(), #name, [this]()->bool&{return name;})
#define CMDLIME_EXITFLAG(name) bool name = cmdlime::detail::makeFlagCreator(cfgReader(), #name, [this]()->bool&{return name;}, cmdlime::detail::Flag::Type::Exit)
#define CMDLIME_ARG(name, type) type name = cmdlime::detail::makeArgCreator<type>(cfgReader(), #name, #type, [this]()->type&{return name;})
#define CMDLIME_ARGLIST(name, type) std::vector<type> name = cmdlime::detail::makeArgListCreator<type>(cfgReader(), #name, #type, [this]()->std::vector<type>&{return name;})
#define CMDLIME_COMMAND(name, type) cmdlime::detail::InitializedOptional<type> name = cmdlime::detail::makeCommandCreator<type>(cfgReader(), #name, [this]()->cmdlime::detail::InitializedOptional<type>&{return name;})
#define CMDLIME_SUBCOMMAND(name, type) cmdlime::detail::InitializedOptional<type> name = cmdlime::detail::makeCommandCreator<type>(cfgReader(), #name, [this]()->cmdlime::detail::InitializedOptional<type>&{return name;}, cmdlime::detail::Command<type>::Type::SubCommand)
