#pragma once
#include <vector>
#include <optional>

#define CMDLIME_PARAM(name, type) type name = param<&std::remove_pointer_t<decltype(this)>::name>(#name, #type)
#define CMDLIME_PARAMLIST(name, type) type name = paramList<&std::remove_pointer_t<decltype(this)>::name>(#name, #type)
#define CMDLIME_FLAG(name) bool name = flag<&std::remove_pointer_t<decltype(this)>::name>(#name)
#define CMDLIME_EXITFLAG(name) bool name = exitFlag<&std::remove_pointer_t<decltype(this)>::name>(#name)
#define CMDLIME_ARG(name, type) type name = arg<&std::remove_pointer_t<decltype(this)>::name>(#name, #type)
#define CMDLIME_ARGLIST(name, type) type name = argList<&std::remove_pointer_t<decltype(this)>::name>(#name, #type)
#define CMDLIME_COMMAND(name, type) cmdlime::detail::InitializedOptional<type> name = command<&std::remove_pointer_t<decltype(this)>::name>(#name)
#define CMDLIME_SUBCOMMAND(name, type) cmdlime::detail::InitializedOptional<type> name = subCommand<&std::remove_pointer_t<decltype(this)>::name>(#name)
