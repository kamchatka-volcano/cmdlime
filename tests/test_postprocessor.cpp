#include "assert_exception.h"
#include <cmdlime/commandlinereader.h>
#include <cmdlime/config.h>
#include <cmdlime/detail/external/sfun/string_utils.h>
#include <cmdlime/postprocessor.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <optional>

namespace test_postprocessor {

using namespace cmdlime;

struct NestedSubcommandConfig : public Config {
    CMDLIME_PARAM(prm, std::string);
};

struct SubcommandConfig : public Config {
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_COMMAND(nested, NestedSubcommandConfig);
};

struct FullConfig : public Config {
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(name, cmdlime::optional<std::string>);
    CMDLIME_PARAM(alias, cmdlime::optional<std::string>);
    CMDLIME_PARAMLIST(prmList, std::vector<std::string>);
    CMDLIME_ARG(argument, double);
    CMDLIME_COMMAND(cmd, SubcommandConfig);
    CMDLIME_SUBCOMMAND(subcommand, SubcommandConfig);
};

} //namespace test_postprocessor
namespace cmdlime {
template<>
struct PostProcessor<test_postprocessor::FullConfig> {
    void operator()(test_postprocessor::FullConfig& cfg)
    {
        std::transform(
                cfg.requiredParam.begin(),
                cfg.requiredParam.end(),
                cfg.requiredParam.begin(),
                [](const auto& ch)
                {
                    return sfun::toupper(ch);
                });
        if (!cfg.name.has_value() && !cfg.alias.has_value())
            throw cmdlime::ValidationError{"either name or alias parameter must be set."};
    }
};

} //namespace cmdlime

namespace test_postprocessor {

TEST(TestPostProcessor, AllSet)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    auto cfg = reader.read<FullConfig>({"-r", "foo", "-nBAR", "-p", "zero", "-p", "one", "4.2"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.name, std::string{"BAR"});
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_FALSE(cfg.cmd);
    EXPECT_FALSE(cfg.subcommand);
}

TEST(TestPostProcessor, InvalidConfig)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]
            {
                reader.read<FullConfig>({"-r", "foo", "-p", "zero", "-p", "one", "4.2"});
            },
            [](const cmdlime::ParsingError& error)
            {
                EXPECT_EQ(
                        std::string{error.what()},
                        std::string{"Command line is invalid: either name or alias parameter must be set."});
            });
}

} //namespace test_postprocessor
