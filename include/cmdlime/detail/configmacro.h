#pragma once
#include "param.h"
#include "paramlist.h"
#include "flag.h"
#include "arg.h"
#include "arglist.h"

#define PARAM(name, type) type name = cmdlime::detail::ParamCreator<type, std::remove_reference_t<decltype(*this)>>{*this, #name, #type, [this]()->type&{return name;}}
#define PARAMLIST(name, type) std::vector<type> name = cmdlime::detail::ParamListCreator<type, std::remove_reference_t<decltype(*this)>>{*this, #name, #type, [this]()->std::vector<type>&{return name;}}
#define FLAG(name) bool name = cmdlime::detail::FlagCreator<std::remove_reference_t<decltype(*this)>>{*this, #name, [this]()->bool&{return name;}}
#define ARG(name, type) type name = cmdlime::detail::ArgCreator<type, std::remove_reference_t<decltype(*this)>>{*this, #name, #type, [this]()->type&{return name;}}
#define ARGLIST(name, type) std::vector<type> name = cmdlime::detail::ArgListCreator<type, std::remove_reference_t<decltype(*this)>>{*this, #name, #type, [this]()->std::vector<type>&{return name;}}

