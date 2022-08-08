#include <gtest/gtest.h>
#include <cmdlime/config.h>
#include <cmdlime/configreader.h>
#include "assert_exception.h"
#include <optional>

#if __has_include(<nameof.hpp>)
#define NAMEOF_AVAILABLE
#endif


namespace test_gnu_format{

using namespace cmdlime;

struct NestedSubcommandConfig: public Config{
    CMDLIME_PARAM(prm, std::string);
};

struct SubcommandConfig: public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i");
    CMDLIME_PARAMLIST(prmList, std::string) << cmdlime::ShortName("L");
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100}) << cmdlime::ShortName("O");
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, float);
    CMDLIME_COMMAND(nested, NestedSubcommandConfig);
};


struct FullConfig : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalParam2, std::optional<std::string>) << cmdlime::WithoutShortName{};
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i");
    CMDLIME_PARAMLIST(prmList, std::string) << cmdlime::ShortName("L");
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100}) << cmdlime::ShortName("O");
    CMDLIME_FLAG(flg);
    CMDLIME_FLAG(secondFlag) << cmdlime::WithoutShortName();
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, float);
    CMDLIME_COMMAND(cmd, SubcommandConfig);
    CMDLIME_SUBCOMMAND(subcommand, SubcommandConfig);
};

#ifdef CMDLIME_NAMEOF_AVAILABLE
struct FullConfigWithoutMacro : public Config{
    std::string requiredParam           = param<&T::requiredParam>();
    std::string optionalParam           = param<&T::optionalParam>()("defaultValue");
    std::optional<std::string> optionalParam2 = param<&T::optionalParam2>() << cmdlime::WithoutShortName{};
    std::optional<int> optionalIntParam = param<&T::optionalIntParam>()() << cmdlime::ShortName("i");
    std::vector<std::string> prmList    = paramList<&T::prmList>() << cmdlime::ShortName("L");
    std::vector<int> optionalPrmList    = paramList<&T::optionalPrmList>()(std::vector<int>{99, 100}) << cmdlime::ShortName("O");
    bool flg = flag<&T::flg>();
    double a = arg<&T::a>();
    std::vector<float> aList = argList<&T::aList>();
    cmdlime::optional<SubcommandConfig> cmd = command<&T::cmd>();
    cmdlime::optional<SubcommandConfig> subcommand = subCommand<&T::subcommand>();

private:
    using T = FullConfigWithoutMacro;
};
#else
struct FullConfigWithoutMacro : public Config{
    std::string requiredParam           = param<&T::requiredParam>("requiredParam", "string");
    std::string optionalParam           = param<&T::optionalParam>("optionalParam", "string")("defaultValue");
    std::optional<std::string> optionalParam2 = param<&T::optionalParam2>("optionalParam2", "string") << cmdlime::WithoutShortName{};
    std::optional<int> optionalIntParam = param<&T::optionalIntParam>("optionalIntParam", "int")() << cmdlime::ShortName("i");
    std::vector<std::string> prmList    = paramList<&T::prmList>("prmList", "string") << cmdlime::ShortName("L");
    std::vector<int> optionalPrmList    = paramList<&T::optionalPrmList>("optionalPrmList", "int")(std::vector<int>{99, 100}) << cmdlime::ShortName("O");
    bool flg = flag<&T::flg>("flg");
    double a = arg<&T::a>("a", "double");
    std::vector<float> aList = argList<&T::aList>("aList", "float");
    cmdlime::optional<SubcommandConfig> cmd = command<&T::cmd>("cmd");
    cmdlime::optional<SubcommandConfig> subcommand = subCommand<&T::subcommand>("subcommand");

private:
    using T = FullConfigWithoutMacro;
};
#endif

TEST(GNUConfig, AllSet)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfig>(
            {"-r", "FOO", "-oBAR", "--optional-param2", "Hello world", "--optional-int-param", "-9", "-L", "zero", "-L", "one",
             "--optional-param-list=1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    ASSERT_TRUE(cfg.optionalParam2.has_value());
    EXPECT_EQ(*cfg.optionalParam2, "Hello world");
    EXPECT_EQ(cfg.optionalIntParam, -9);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flg, true);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    EXPECT_FALSE(cfg.cmd);
    EXPECT_FALSE(cfg.subcommand);
}

TEST(GNUConfig, AllSetWithoutMacro)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfigWithoutMacro>(
            {"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
             "--optional-prm-list=1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalPrmList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flg, true);
    EXPECT_EQ(cfg.a, 4.2);
    EXPECT_EQ(cfg.aList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    EXPECT_FALSE(cfg.cmd);
    EXPECT_FALSE(cfg.subcommand);
}

TEST(GNUConfig, AllSetInSubCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfig>({"-r", "FOO", "--prm-list=zero", "--prm-list=one", "4.2", "1.1",
                                           "subcommand", "--required-param", "FOO", "--optional-param=BAR",
                                           "--optional-int-param", "9", "--prm-list=zero", "--prm-list=one",
                                           "--optional-param-list=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f}));
    EXPECT_FALSE(cfg.cmd);
    ASSERT_TRUE(cfg.subcommand);
    EXPECT_EQ(cfg.subcommand->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.subcommand->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.subcommand->optionalIntParam, 9);
    EXPECT_EQ(cfg.subcommand->prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.subcommand->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.subcommand->flg, true);
    EXPECT_EQ(cfg.subcommand->argument, 4.2);
    EXPECT_EQ(cfg.subcommand->argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, AllSetInSubCommandWithoutMacro)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfigWithoutMacro>({"-r", "FOO", "--prm-list=zero", "--prm-list=one", "4.2", "1.1",
                                                       "subcommand", "--required-param", "FOO", "--optional-param=BAR",
                                                       "--optional-int-param", "9", "--prm-list=zero",
                                                       "--prm-list=one",
                                                       "--optional-param-list=1,2", "--flg", "4.2", "1.1", "2.2",
                                                       "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalPrmList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.a, 4.2);
    EXPECT_EQ(cfg.aList, (std::vector<float>{1.1f}));
    EXPECT_FALSE(cfg.cmd);
    ASSERT_TRUE(cfg.subcommand);
    EXPECT_EQ(cfg.subcommand->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.subcommand->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.subcommand->optionalIntParam, 9);
    EXPECT_EQ(cfg.subcommand->prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.subcommand->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.subcommand->flg, true);
    EXPECT_EQ(cfg.subcommand->argument, 4.2);
    EXPECT_EQ(cfg.subcommand->argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, AllSetInCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfig>(
            {"cmd", "--required-param=FOO", "--optional-param=BAR", "--optional-int-param=9", "--prm-list=zero",
             "--prm-list=one",
             "--optional-param-list=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_FALSE(cfg.subcommand);
    ASSERT_TRUE(cfg.cmd);
    EXPECT_EQ(cfg.cmd->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.cmd->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.cmd->optionalIntParam, 9);
    EXPECT_EQ(cfg.cmd->prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.cmd->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.cmd->flg, true);
    EXPECT_EQ(cfg.cmd->argument, 4.2);
    EXPECT_EQ(cfg.cmd->argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, AllSetInCommandWithoutMacro)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfigWithoutMacro>(
            {"cmd", "--required-param", "FOO", "--optional-param=BAR", "--optional-int-param", "9", "--prm-list=zero",
             "--prm-list=one",
             "--optional-param-list=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_FALSE(cfg.subcommand);
    ASSERT_TRUE(cfg.cmd);
    EXPECT_EQ(cfg.cmd->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.cmd->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.cmd->optionalIntParam, 9);
    EXPECT_EQ(cfg.cmd->prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.cmd->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.cmd->flg, true);
    EXPECT_EQ(cfg.cmd->argument, 4.2);
    EXPECT_EQ(cfg.cmd->argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}


TEST(GNUConfig, CombinedFlagsAndParams)
{
    struct Cfg : public Config{
        CMDLIME_FLAG(firstFlag) << cmdlime::ShortName("f");
        CMDLIME_FLAG(secondFlag) << cmdlime::ShortName("s");
        CMDLIME_FLAG(thirdFlag) << cmdlime::ShortName("t");
        CMDLIME_PARAM(prm, std::string)() << cmdlime::ShortName("p");
    };

    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<Cfg>({"-fst", "-pfirst"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.prm, "first");
    }

    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<Cfg>({"-tfspfirst"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.prm, "first");
    }

    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<Cfg>({"-fs", "-tp" "first"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.prm, "first");
    }

    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<Cfg>({"-fs", "--prm", "first"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, false);
    EXPECT_EQ(cfg.prm, "first");
    }
}

TEST(GNUConfig, NumericParamsAndFlags)
{
    struct Cfg : public Config{
        CMDLIME_FLAG(flg) << cmdlime::ShortName("1");
        CMDLIME_PARAM(prm, std::string) << cmdlime::ShortName("2");
        CMDLIME_PARAM(paramSecond, std::string)("default") << cmdlime::ShortName("p");
        CMDLIME_ARG(argument, int);
    };
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        auto cfg = cfgReader.read<Cfg>({"-1", "-2", "test", "-99"});
        EXPECT_EQ(cfg.flg, true);
        EXPECT_EQ(cfg.prm, "test");
        EXPECT_EQ(cfg.paramSecond, "default");
        EXPECT_EQ(cfg.argument, -99);
    }
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        auto cfg = cfgReader.read<Cfg>({"-2test", "-1", "--param-second=second", "-99"});
        EXPECT_EQ(cfg.flg, true);
        EXPECT_EQ(cfg.prm, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.argument, -99);
    }
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        auto cfg = cfgReader.read<Cfg>({"-12test", "--param-second", "second", "-99"});
        EXPECT_EQ(cfg.flg, true);
        EXPECT_EQ(cfg.prm, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.argument, -99);
    }
}

TEST(GNUConfig, MissingOptionals)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfig>({"-r", "FOO", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}


TEST(GNUConfig, MissingParamAllSetInSubCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"subcommand", "--required-param=FOO", "--optional-param=BAR", "--optional-int-param=9", "--prm-list",
                 "zero", "--prm-list=one",
                 "--optional-param-list=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--required-param' is missing."});
        });
}

TEST(GNUConfig, MissingParamAllSetInCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfig>(
            {"cmd", "--required-param=FOO", "--optional-param=BAR", "--optional-int-param=9", "--prm-list=zero",
             "--prm-list=one",
             "--optional-param-list=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.prmList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 0.f);
    EXPECT_TRUE(cfg.argumentList.empty());
    ASSERT_TRUE(cfg.cmd);
    EXPECT_EQ(cfg.cmd->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.cmd->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.cmd->optionalIntParam, 9);
    EXPECT_EQ(cfg.cmd->prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.cmd->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.cmd->flg, true);
    EXPECT_EQ(cfg.cmd->argument, 4.2);
    EXPECT_EQ(cfg.cmd->argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, MissingParamAllSetInNestedCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfig>({"cmd", "nested", "--prm=FOO"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.prmList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 0.f);
    EXPECT_TRUE(cfg.argumentList.empty());
    ASSERT_TRUE(cfg.cmd);
    EXPECT_TRUE(cfg.cmd->requiredParam.empty());
    EXPECT_EQ(cfg.cmd->optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.cmd->optionalIntParam.has_value());
    EXPECT_TRUE(cfg.cmd->prmList.empty());
    EXPECT_EQ(cfg.cmd->optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.cmd->flg, false);
    EXPECT_EQ(cfg.cmd->argument, 0.f);
    EXPECT_TRUE(cfg.cmd->argumentList.empty());
    ASSERT_TRUE(cfg.cmd->nested);
    EXPECT_EQ(cfg.cmd->nested->prm, "FOO");
}

struct FullConfigWithOptionalArgList : public Config{
    CMDLIME_PARAM(requiredParam, std::string) << cmdlime::ShortName("r");
    CMDLIME_PARAM(optionalParam, std::string)({"defaultValue"}) << cmdlime::ShortName("o");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i");
    CMDLIME_FLAG(flg) << cmdlime::ShortName("f");
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, float)({1.f, 2.f});
};

TEST(GNUConfig, MissingOptionalArgList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<FullConfigWithOptionalArgList>({"-r", "FOO", "4.2"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.f, 2.f}));
}

TEST(GNUConfig, MissingParam)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>({"-o", "FOO", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--required-param' is missing."});
        });
}

TEST(GNUConfig, MissingArg)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfigWithOptionalArgList>({"-r", "FOO", "-o", "BAR", "-i", "9", "-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'argument' is missing."});
        });
}

TEST(GNUConfig, MissingArgList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>({"-r", "FOO", "-o", "BAR", "-i", "9", "-L", "zero", "-f", "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'argument-list' is missing."});
        });
}

TEST(GNUConfig, MissingParamList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "-i", "9", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--prm-list' is missing."});
        });
}

TEST(GNUConfig, UnexpectedParam)
{
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-r", "FOO", "--test", "TEST", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '--test'"});
        });
    }
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>({"-r", "FOO", "-t", "TEST", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-t'"});
        });
    }
}

TEST(GNUConfig, UnexpectedFlag)
{
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>({"-r", "FOO", "--test", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '--test'"});
        });
    }
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>({"-r", "FOO", "-t", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-t'"});
        });
    }
}

TEST(GNUConfig, UnexpectedArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::ShortName("p");
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-p", "FOO", "4.2", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
        });
}

TEST(GNUConfig, WrongParamType)
{
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "-i", "nine", "-L", "zero", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-int-param' value from 'nine'"});
        });
    }
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "--optional-int-param", "nine", "-L", "zero", "-f", "4.2", "1.1", "2.2",
                 "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-int-param' value from 'nine'"});
        });
    }
}

TEST(GNUConfig, WrongParamListElementType)
{
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        assert_exception<cmdlime::ParsingError>(
                    [&]{auto cfg = cfgReader.read<FullConfig>(
                            {"-r", "FOO", "-o", "BAR", "-L", "zero", "-O", "not-int", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-param-list' value from 'not-int'"});
        });
    }
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        assert_exception<cmdlime::ParsingError>(
                    [&]{auto cfg = cfgReader.read<FullConfig>(
                            {"-r", "FOO", "-o", "BAR", "-L", "zero", "--optional-param-list=not-int", "-f", "4.2",
                             "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-param-list' value from 'not-int'"});
        });
    }
}

TEST(GNUConfig, WrongArgType)
{
   auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "-i", "9", "-L", "zero", "-f", "fortytwo", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'argument' value from 'fortytwo'"});
        });
}

TEST(GNUConfig, WrongArgListElementType)
{
   auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "-i", "9", "-L", "zero", "-f", "4.2", "1.1", "2.2", "three"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument list 'argument-list' element's value from 'three'"});
        });
}

TEST(GNUConfig, ParamEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::ShortName("p");
        CMDLIME_FLAG(flg) << cmdlime::ShortName("f");
    };
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
    }
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"--prm"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--prm' value can't be empty"});
        });
    }
}

TEST(GNUConfig, ParamListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAMLIST(params, std::string) << cmdlime::ShortName("p");
    };
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        assert_exception<cmdlime::ParsingError>(
                    [&]{auto cfg = cfgReader.read<Cfg>({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
    }
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        assert_exception<cmdlime::ParsingError>(
            [&]{auto cfg = cfgReader.read<Cfg>({"--params"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--params' value can't be empty"});
            });
    }
}

TEST(GNUConfig, ArgEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARG(argument, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg 'argument' value can't be empty"});
        });
}

TEST(GNUConfig, ArgListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARGLIST(args, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"foo", ""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg list 'args' element value can't be empty"});
        });
}


TEST(GNUConfig, ValuesWithWhitespace)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::ShortName("p");
        CMDLIME_PARAMLIST(prmList, std::string) << cmdlime::ShortName("L");
        CMDLIME_ARG(argument, std::string);
        CMDLIME_ARGLIST(argumentList, std::string);
    };
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        auto cfg = cfgReader.read<Cfg>({"-p", "Hello world", "-L", "foo bar", "foo bar", "forty two"});
        EXPECT_EQ(cfg.prm, "Hello world");
        EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"foo bar"}));
        EXPECT_EQ(cfg.argument, "foo bar");
        EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"forty two"}));
    }
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        auto cfg = cfgReader.read<Cfg>({"--prm=Hello world", "--prm-list", "foo bar", "foo bar", "forty two"});
        EXPECT_EQ(cfg.prm, "Hello world");
        EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"foo bar"}));
        EXPECT_EQ(cfg.argument, "foo bar");
        EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"forty two"}));
    }
}

TEST(GNUConfig, ParamWrongNameNonAlphaFirstChar)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::Name("!param");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name '!param' must start with an alphabet character"});
        });
}

TEST(GNUConfig, ParamWrongNameNonAlphanum)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::Name("p$r$m");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'p$r$m' must consist of alphanumeric characters and hyphens"});
        });
}

TEST(GNUConfig, ParamWrongShortNameTooLong)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(parameter, std::string) << cmdlime::ShortName("prm");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's short name 'prm' can't have more than one symbol"});
        });
}

TEST(GNUConfig, ParamWrongShortNameNonAlphanum)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::ShortName("$");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's short name '$' must be an alphanumeric character"});
        });
}

TEST(GNUConfig, NegativeNumberToArg)
{
    struct Cfg : public Config{
        CMDLIME_ARG(argument, int);
        CMDLIME_ARG(argumentStr, std::string);
        CMDLIME_ARGLIST(argumentList, double);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<Cfg>({"-2", "-3", "4.5", "-6.7"});
    EXPECT_EQ(cfg.argument, -2);
    EXPECT_EQ(cfg.argumentStr, "-3");
    EXPECT_EQ(cfg.argumentList, (std::vector<double>{4.5, -6.7}));
}

TEST(GNUConfig, NegativeNumberWithoutArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, int) << cmdlime::ShortName("p");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<Cfg>({"-2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '-2'"});
        });
}

TEST(GNUConfig, ArgsDelimiter)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, int) << cmdlime::ShortName("p");
        CMDLIME_PARAM(optionalParam, int)(0) << cmdlime::ShortName("o");
        CMDLIME_ARGLIST(argumentList, std::string);
    };

    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        auto cfg = cfgReader.read<Cfg>({"-p", "11", "0", "--", "1", "-o", "2"});
        EXPECT_EQ(cfg.prm, 11);
        EXPECT_EQ(cfg.optionalParam, 0);
        EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "-o", "2"}));
    }
    {
        auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
        auto cfg = cfgReader.read<Cfg>({"--prm", "11", "0", "--", "1", "-o", "2"});
        EXPECT_EQ(cfg.prm, 11);
        EXPECT_EQ(cfg.optionalParam, 0);
        EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "-o", "2"}));
    }
}

TEST(GNUConfig, ArgsDelimiterFront)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0) << cmdlime::ShortName("o");
        CMDLIME_ARGLIST(argumentList, std::string);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<Cfg>({"--", "0", "1", "-o", "1", "2"});
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "-o", "1", "2"}));
}

TEST(GNUConfig, ArgsDelimiterBack)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0) << cmdlime::ShortName("o");
        CMDLIME_ARGLIST(argumentList, std::string);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<Cfg>({"-o", "1", "0", "1", "2", "--"});
    EXPECT_EQ(cfg.optionalParam, 1);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "2"}));
}

TEST(GNUConfig, PascalNames)
{
    struct PascalConfig : public Config{
        CMDLIME_PARAM(RequiredParam, std::string);
        CMDLIME_PARAM(OptionalParam, std::string)("defaultValue");
        CMDLIME_PARAM(IntParamOptional, std::optional<int>)();
        CMDLIME_PARAMLIST(ListOfParam, std::string);
        CMDLIME_PARAMLIST(MyListOfParamOptional, int)(std::vector<int>{99, 100});
        CMDLIME_FLAG(flg);
        CMDLIME_ARG(argument, double);
        CMDLIME_ARGLIST(argumentList, float);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<PascalConfig>(
            {"--required-param", "FOO", "--optional-param", "BAR", "--int-param-optional", "9", "--list-of-param",
             "zero", "--list-of-param", "one",
             "--my-list-of-param-optional", "1", "--my-list-of-param-optional", "2", "--flg", "4.2", "1.1", "2.2",
             "3.3"});
    EXPECT_EQ(cfg.RequiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.OptionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.IntParamOptional, 9);
    EXPECT_EQ(cfg.ListOfParam, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.MyListOfParamOptional, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flg, true);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, CustomNamesWithoutShortName)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(requiredParam, std::string) << cmdlime::WithoutShortName{};
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue") << cmdlime::WithoutShortName{};
        CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::WithoutShortName{};
        CMDLIME_PARAMLIST(prmList, std::string) << cmdlime::WithoutShortName{};
        CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100}) << cmdlime::WithoutShortName{};
        CMDLIME_FLAG(flg) << cmdlime::WithoutShortName{};
        CMDLIME_FLAG(secondFlag);
        CMDLIME_ARG(argument, double);
        CMDLIME_ARGLIST(argumentList, float);
    };

    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<TestConfig>(
            {"--required-param", "FOO", "--optional-param", "BAR", "--optional-int-param", "9", "--prm-list", "zero",
             "--prm-list", "one",
             "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flg, true);
    EXPECT_EQ(cfg.secondFlag, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    }

    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({"-r"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-r'"});
        });
    }
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({"-o"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-o'"});
        });
    }
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({"-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-f'"});
        });
    }
    {
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    cfgReader.setProgramName("testproc");
    auto expectedDetailedInfo = std::string{
    "Usage: testproc <argument> --required-param <string> --prm-list <string>... [params] [flags] <argument-list...>\n"
    "Arguments:\n"
    "    <argument> (double)               \n"
    "    <argument-list> (float)           multi-value\n"
    "Parameters:\n"
    "       --required-param <string>      \n"
    "       --prm-list <string>            multi-value\n"
    "       --optional-param <string>      optional, default: defaultValue\n"
    "       --optional-int-param <int>     optional\n"
    "       --optional-param-list <int>    multi-value, optional, default: {99, \n"
    "                                        100}\n"
    "Flags:\n"
    "       --flg                          \n"
    "   -s, --second-flag                  \n"};
    EXPECT_EQ(cfgReader.usageInfoDetailed<TestConfig>(), expectedDetailedInfo);
    }
}

TEST(GNUConfig, CustomNamesMissingParam)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(prm, double) << cmdlime::Name{"P"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<TestConfig>({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--P' is missing."});
        });
}

TEST(GNUConfig, CustomNamesMissingParamList)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(prm, double) << cmdlime::ShortName{"P"};
        CMDLIME_PARAMLIST(prmList, float) << cmdlime::Name{"L"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({"-P1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--L' is missing."});
        });
}

TEST(GNUConfig, CustomNamesMissingArg)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(argument, double) << cmdlime::Name{"Argument"};
        CMDLIME_ARGLIST(argumentList, float) << cmdlime::Name{"ArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'Argument' is missing."});
        });
}

TEST(GNUConfig, CustomNamesMissingArgList)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(argument, double) << cmdlime::Name{"Argument"};
        CMDLIME_ARGLIST(argumentList, float) << cmdlime::Name{"ArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({"1.0"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'ArgList' is missing."});
        });
}

TEST(GNUConfig, ConfigErrorRepeatingParamNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(Prm, double)();
        CMDLIME_PARAM(prm, int)();
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ cfgReader.read<TestConfig>({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'prm' is already used."});
        });
}

TEST(GNUConfig, ConfigErrorRepeatingParamShortNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(prm, double)();
        CMDLIME_PARAMLIST(prmList, int)();
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ cfgReader.read<TestConfig>({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's short name 'p' is already used."});
        });
}


TEST(GNUConfig, UsageInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    cfgReader.setProgramName("testproc");
    auto expectedInfo = std::string{
    "Usage: testproc [commands] <argument> --required-param <string> --prm-list <string>... "
    "[--optional-param <string>] [--optional-param2 <string>] [--optional-int-param <int>] [--optional-param-list <int>...] [--flg] [--second-flag] <argument-list...>\n"
    };
    EXPECT_EQ(cfgReader.usageInfo<FullConfig>(), expectedInfo);
}

TEST(GNUConfig, UsageInfoWithoutMacro)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    cfgReader.setProgramName("testproc");
    auto expectedInfo = std::string{
            "Usage: testproc [commands] <a> --required-param <string> --prm-list <string>... "
            "[--optional-param <string>] [--optional-param2 <string>] [--optional-int-param <int>] [--optional-prm-list <int>...] [--flg] <a-list...>\n"
    };
    EXPECT_EQ(cfgReader.usageInfo<FullConfigWithoutMacro>(), expectedInfo);
}

TEST(GNUConfig, DetailedUsageInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    cfgReader.setProgramName("testproc");
    auto expectedDetailedInfo = std::string{
    "Usage: testproc [commands] <argument> --required-param <string> --prm-list <string>... [params] [flags] <argument-list...>\n"
    "Arguments:\n"
    "    <argument> (double)               \n"
    "    <argument-list> (float)           multi-value\n"
    "Parameters:\n"
    "   -r, --required-param <string>      \n"
    "   -L, --prm-list <string>            multi-value\n"
    "   -o, --optional-param <string>      optional, default: defaultValue\n"
    "       --optional-param2 <string>     optional\n"
    "   -i, --optional-int-param <int>     optional\n"
    "   -O, --optional-param-list <int>    multi-value, optional, default: {99, \n"
    "                                        100}\n"
    "Flags:\n"
    "   -f, --flg                          \n"
    "       --second-flag                  \n"
    "Commands:\n"
    "    cmd [options]                     \n"
    "    subcommand [options]              \n"};
    EXPECT_EQ(cfgReader.usageInfoDetailed<FullConfig>(), expectedDetailedInfo);
}

TEST(GNUConfig, DetailedUsageInfoWithoutMacro)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    cfgReader.setProgramName("testproc");
    auto expectedDetailedInfo = std::string{
            "Usage: testproc [commands] <a> --required-param <string> --prm-list <string>... [params] [flags] <a-list...>\n"
            "Arguments:\n"
            "    <a> (double)                      \n"
            "    <a-list> (float)                  multi-value\n"
            "Parameters:\n"
            "   -r, --required-param <string>      \n"
            "   -L, --prm-list <string>            multi-value\n"
            "   -o, --optional-param <string>      optional, default: defaultValue\n"
            "       --optional-param2 <string>     optional\n"
            "   -i, --optional-int-param <int>     optional\n"
            "   -O, --optional-prm-list <int>      multi-value, optional, default: {99, \n"
            "                                        100}\n"
            "Flags:\n"
            "   -f, --flg                          \n"
            "Commands:\n"
            "    cmd [options]                     \n"
            "    subcommand [options]              \n"};
    EXPECT_EQ(cfgReader.usageInfoDetailed<FullConfigWithoutMacro>(), expectedDetailedInfo);
}


TEST(GNUConfig, WrongParamsWithExitFlag){
    struct ConfigWithExitFlag : public Config{
        CMDLIME_PARAM(requiredParam, int);
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
        CMDLIME_FLAG(flg);
        CMDLIME_EXITFLAG(exitFlg);
        CMDLIME_ARG(argument, double);
        CMDLIME_ARGLIST(argumentList, float);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<ConfigWithExitFlag>({"-asd", "asf", "--exit-flg"});
    EXPECT_EQ(cfg.exitFlg, true);
}

TEST(GNUConfig, InvalidParamsWithExitFlag){
    struct ConfigWithExitFlag : public Config{
        CMDLIME_PARAM(requiredParam, int)(0) << [](int val){ if (val < 1) throw cmdlime::ValidationError("Value must be greater than 0"); };
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
        CMDLIME_FLAG(flg);
        CMDLIME_EXITFLAG(exitFlg);
        CMDLIME_ARG(argument, double);
        CMDLIME_ARGLIST(argumentList, float);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<ConfigWithExitFlag>({"-asd", "asf", "--exit-flg"});
    EXPECT_EQ(cfg.exitFlg, true);
}


struct CustomType{
    std::string value;
};

struct CustomTypeConfig: public Config{
    CMDLIME_PARAM(prm, CustomType);
    CMDLIME_PARAMLIST(prmList, CustomType) << cmdlime::ShortName{"l"};
    CMDLIME_ARG(argument, CustomType);
    CMDLIME_ARGLIST(argumentList, CustomType);
};

struct CustomTypeInt{
    int value;
};

struct CustomTypeIntConfig: public Config{
    CMDLIME_PARAM(prm, CustomTypeInt);
    CMDLIME_PARAMLIST(prmList, CustomTypeInt) << cmdlime::ShortName{"l"};
    CMDLIME_ARG(argument, CustomTypeInt);
    CMDLIME_ARGLIST(argumentList, CustomTypeInt);
};


TEST(GNUConfig, CustomTypeUsage){
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<CustomTypeConfig>(
            {"--prm", "hello world", "test arg", "--prm-list", "foo bar", "--prm-list", "baz", "1", "2 3"});
    EXPECT_EQ(cfg.prm.value, "hello world");
    ASSERT_EQ(cfg.prmList.size(), 2); //(std::vector<CustomType>{{"foo bar"}, {"baz"}}));
    EXPECT_EQ(cfg.prmList.at(0).value, "foo bar");
    EXPECT_EQ(cfg.prmList.at(1).value, "baz");
    EXPECT_EQ(cfg.argument.value, "test arg");
    EXPECT_EQ(cfg.argumentList.size(), 2);
    EXPECT_EQ(cfg.argumentList.at(0).value, "1");
    EXPECT_EQ(cfg.argumentList.at(1).value, "2 3");
}

TEST(GNUConfig, CustomTypeIntUsage){
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::GNU>{};
    auto cfg = cfgReader.read<CustomTypeIntConfig>(
            {"--prm", "10", "42", "--prm-list", "0", "--prm-list", "1", "43", "44"});
    EXPECT_EQ(cfg.prm.value, 10);
    ASSERT_EQ(cfg.prmList.size(), 2); //(std::vector<CustomType>{{"foo bar"}, {"baz"}}));
    EXPECT_EQ(cfg.prmList.at(0).value, 0);
    EXPECT_EQ(cfg.prmList.at(1).value, 1);
    EXPECT_EQ(cfg.argument.value, 42);
    EXPECT_EQ(cfg.argumentList.size(), 2);
    EXPECT_EQ(cfg.argumentList.at(0).value, 43);
    EXPECT_EQ(cfg.argumentList.at(1).value, 44);
}


}

namespace cmdlime{
    template<>
    struct StringConverter<test_gnu_format::CustomType>{
        static std::string toString(const test_gnu_format::CustomType& val)
        {
            return val.value;
        }

        static std::optional<test_gnu_format::CustomType> fromString(const std::string& str)
        {
            auto val = test_gnu_format::CustomType{};
            val.value = str;
            return val;
        }
    };

template<>
struct StringConverter<test_gnu_format::CustomTypeInt>{
    static std::string toString(const test_gnu_format::CustomTypeInt& val)
    {
        return std::to_string(val.value);
    }

    static std::optional<test_gnu_format::CustomTypeInt> fromString(const std::string& str)
    {
        auto val = test_gnu_format::CustomTypeInt{};
        val.value = std::stoi(str);
        return val;
    }
};
}