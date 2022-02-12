#include <gtest/gtest.h>
#include <cmdlime/gnuconfig.h>
#include "assert_exception.h"
#include <optional>

#if __has_include(<nameof.hpp>)
#define NAMEOF_AVAILABLE
#endif


namespace test_gnu_format{

using Config = cmdlime::GNUConfig;

struct NestedSubcommandConfig: public Config{
    CMDLIME_PARAM(param, std::string);
};

struct SubcommandConfig: public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i");
    CMDLIME_PARAMLIST(paramList, std::string) << cmdlime::ShortName("L");
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100}) << cmdlime::ShortName("O");
    CMDLIME_FLAG(flag);
    CMDLIME_ARG(arg, double);
    CMDLIME_ARGLIST(argList, float);
    CMDLIME_COMMAND(nested, NestedSubcommandConfig);
};


struct FullConfig : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i");
    CMDLIME_PARAMLIST(paramList, std::string) << cmdlime::ShortName("L");
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100}) << cmdlime::ShortName("O");
    CMDLIME_FLAG(flag);
    CMDLIME_FLAG(secondFlag) << cmdlime::WithoutShortName();
    CMDLIME_ARG(arg, double);
    CMDLIME_ARGLIST(argList, float);
    CMDLIME_COMMAND(command, SubcommandConfig);
    CMDLIME_SUBCOMMAND(subcommand, SubcommandConfig);
};

#ifdef NAMEOF_AVAILABLE
struct FullConfigWithoutMacro : public Config{
    std::string requiredParam           = param<&T::requiredParam>();
    std::string optionalParam           = param<&T::optionalParam>()("defaultValue");
    std::optional<int> optionalIntParam = param<&T::optionalIntParam>()() << cmdlime::ShortName("i");
    std::vector<std::string> prmList    = paramList<&T::prmList>() << cmdlime::ShortName("L");
    std::vector<int> optionalPrmList    = paramList<&T::optionalPrmList>()(std::vector<int>{99, 100}) << cmdlime::ShortName("O");
    bool flg = flag<&T::flg>();
    double a = arg<&T::a>();
    std::vector<float> aList = argList<&T::aList>();
    std::optional<SubcommandConfig> cmd = command<&T::cmd>();
    std::optional<SubcommandConfig> subcommand = subCommand<&T::subcommand>();

private:
    using T = FullConfigWithoutMacro;
};
#else
struct FullConfigWithoutMacro : public Config{
    std::string requiredParam           = param<&T::requiredParam>("requiredParam", "string");
    std::string optionalParam           = param<&T::optionalParam>("optionalParam", "string")("defaultValue");
    std::optional<int> optionalIntParam = param<&T::optionalIntParam>("optionalIntParam", "int")() << cmdlime::ShortName("i");
    std::vector<std::string> prmList    = paramList<&T::prmList>("prmList", "string") << cmdlime::ShortName("L");
    std::vector<int> optionalPrmList    = paramList<&T::optionalPrmList>("optionalPrmList", "int")(std::vector<int>{99, 100}) << cmdlime::ShortName("O");
    bool flg = flag<&T::flg>("flg");
    double a = arg<&T::a>("a", "double");
    std::vector<float> aList = argList<&T::aList>("aList", "float");
    std::optional<SubcommandConfig> cmd = command<&T::cmd>("cmd");
    std::optional<SubcommandConfig> subcommand = subCommand<&T::subcommand>("subcommand");

private:
    using T = FullConfigWithoutMacro;
};
#endif

TEST(GNUConfig, AllSet)
{
    auto cfg = FullConfig{};
    cfg.readCommandLine({"-r", "FOO", "-oBAR", "--optional-int-param", "-9", "-L","zero", "-L", "one",
              "--optional-param-list=1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, -9);
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flag, true);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    EXPECT_FALSE(cfg.command.has_value());
    EXPECT_FALSE(cfg.subcommand.has_value());
}

TEST(GNUConfig, AllSetWithoutMacro)
{
    auto cfg = FullConfigWithoutMacro{};
    cfg.readCommandLine({"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
              "--optional-prm-list=1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalPrmList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flg, true);
    EXPECT_EQ(cfg.a, 4.2);
    EXPECT_EQ(cfg.aList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    EXPECT_FALSE(cfg.cmd.has_value());
    EXPECT_FALSE(cfg.subcommand.has_value());
}

TEST(GNUConfig, AllSetInSubCommand)
{
    auto cfg = FullConfig{};
    cfg.readCommandLine({"-r", "FOO", "--param-list=zero", "--param-list=one", "4.2", "1.1",
              "subcommand", "--required-param", "FOO", "--optional-param=BAR", "--optional-int-param", "9", "--param-list=zero", "--param-list=one",
              "--optional-param-list=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f}));
    EXPECT_FALSE(cfg.command.has_value());
    ASSERT_TRUE(cfg.subcommand.has_value());
    EXPECT_EQ(cfg.subcommand->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.subcommand->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.subcommand->optionalIntParam, 9);
    EXPECT_EQ(cfg.subcommand->paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.subcommand->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.subcommand->flag, true);
    EXPECT_EQ(cfg.subcommand->arg, 4.2);
    EXPECT_EQ(cfg.subcommand->argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, AllSetInSubCommandWithoutMacro)
{
    auto cfg = FullConfigWithoutMacro{};
    cfg.readCommandLine({"-r", "FOO", "--prm-list=zero", "--prm-list=one", "4.2", "1.1",
              "subcommand", "--required-param", "FOO", "--optional-param=BAR", "--optional-int-param", "9", "--param-list=zero", "--param-list=one",
              "--optional-param-list=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalPrmList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.a, 4.2);
    EXPECT_EQ(cfg.aList, (std::vector<float>{1.1f}));
    EXPECT_FALSE(cfg.cmd.has_value());
    ASSERT_TRUE(cfg.subcommand.has_value());
    EXPECT_EQ(cfg.subcommand->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.subcommand->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.subcommand->optionalIntParam, 9);
    EXPECT_EQ(cfg.subcommand->paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.subcommand->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.subcommand->flag, true);
    EXPECT_EQ(cfg.subcommand->arg, 4.2);
    EXPECT_EQ(cfg.subcommand->argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, AllSetInCommand)
{
    auto cfg = FullConfig{};
    cfg.readCommandLine({"command", "--required-param=FOO", "--optional-param=BAR", "--optional-int-param=9", "--param-list=zero", "--param-list=one",
              "--optional-param-list=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_FALSE(cfg.subcommand.has_value());
    ASSERT_TRUE(cfg.command.has_value());
    EXPECT_EQ(cfg.command->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.command->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.command->optionalIntParam, 9);
    EXPECT_EQ(cfg.command->paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.command->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.command->flag, true);
    EXPECT_EQ(cfg.command->arg, 4.2);
    EXPECT_EQ(cfg.command->argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, AllSetInCommandWithoutMacro)
{
    auto cfg = FullConfigWithoutMacro{};
    cfg.readCommandLine({"cmd", "--required-param", "FOO", "--optional-param=BAR", "--optional-int-param", "9", "--param-list=zero", "--param-list=one",
              "--optional-param-list=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_FALSE(cfg.subcommand.has_value());
    ASSERT_TRUE(cfg.cmd.has_value());
    EXPECT_EQ(cfg.cmd->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.cmd->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.cmd->optionalIntParam, 9);
    EXPECT_EQ(cfg.cmd->paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.cmd->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.cmd->flag, true);
    EXPECT_EQ(cfg.cmd->arg, 4.2);
    EXPECT_EQ(cfg.cmd->argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}


TEST(GNUConfig, CombinedFlagsAndParams)
{
    struct Cfg : public Config{
        CMDLIME_FLAG(firstFlag) << cmdlime::ShortName("f");
        CMDLIME_FLAG(secondFlag) << cmdlime::ShortName("s");
        CMDLIME_FLAG(thirdFlag) << cmdlime::ShortName("t");
        CMDLIME_PARAM(param, std::string)() << cmdlime::ShortName("p");
    };

    {
    auto cfg = Cfg{};
    cfg.readCommandLine({"-fst", "-pfirst"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.param, "first");
    }

    {
    auto cfg = Cfg{};
    cfg.readCommandLine({"-tfspfirst"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.param, "first");
    }

    {
    auto cfg = Cfg{};
    cfg.readCommandLine({"-fs","-tp" "first"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.param, "first");
    }

    {
    auto cfg = Cfg{};
    cfg.readCommandLine({"-fs", "--param", "first"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, false);
    EXPECT_EQ(cfg.param, "first");
    }
}

TEST(GNUConfig, NumericParamsAndFlags)
{
    struct Cfg : public Config{
        CMDLIME_FLAG(flag) << cmdlime::ShortName("1");
        CMDLIME_PARAM(param, std::string) << cmdlime::ShortName("2");
        CMDLIME_PARAM(paramSecond, std::string)("default") << cmdlime::ShortName("p");
        CMDLIME_ARG(arg, int);
    };
    {
        auto cfg = Cfg{};
        cfg.readCommandLine({"-1", "-2", "test", "-99"});
        EXPECT_EQ(cfg.flag, true);
        EXPECT_EQ(cfg.param, "test");
        EXPECT_EQ(cfg.paramSecond, "default");
        EXPECT_EQ(cfg.arg, -99);
    }
    {
        auto cfg = Cfg{};
        cfg.readCommandLine({"-2test", "-1", "--param-second=second", "-99"});
        EXPECT_EQ(cfg.flag, true);
        EXPECT_EQ(cfg.param, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.arg, -99);
    }
    {
        auto cfg = Cfg{};
        cfg.readCommandLine({"-12test", "--param-second", "second", "-99"});
        EXPECT_EQ(cfg.flag, true);
        EXPECT_EQ(cfg.param, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.arg, -99);
    }
}

TEST(GNUConfig, MissingOptionals)
{
    auto cfg = FullConfig{};
    cfg.readCommandLine({"-r", "FOO", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}


TEST(GNUConfig, MissingParamAllSetInSubCommand)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"subcommand", "--required-param=FOO", "--optional-param=BAR", "--optional-int-param=9", "--param-list","zero", "--param-list=one",
                         "--optional-param-list=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--required-param' is missing."});
        });
}

TEST(GNUConfig, MissingParamAllSetInCommand)
{
    auto cfg = FullConfig{};
    cfg.readCommandLine({"command", "--required-param=FOO", "--optional-param=BAR", "--optional-int-param=9", "--param-list=zero", "--param-list=one",
              "--optional-param-list=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.paramList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 0.f);
    EXPECT_TRUE(cfg.argList.empty());
    ASSERT_TRUE(cfg.command.has_value());
    EXPECT_EQ(cfg.command->requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.command->optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.command->optionalIntParam, 9);
    EXPECT_EQ(cfg.command->paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.command->optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.command->flag, true);
    EXPECT_EQ(cfg.command->arg, 4.2);
    EXPECT_EQ(cfg.command->argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, MissingParamAllSetInNestedCommand)
{
    auto cfg = FullConfig{};
    cfg.readCommandLine({"command", "nested", "--param=FOO"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.paramList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 0.f);
    EXPECT_TRUE(cfg.argList.empty());
    ASSERT_TRUE(cfg.command.has_value());
    EXPECT_TRUE(cfg.command->requiredParam.empty());
    EXPECT_EQ(cfg.command->optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.command->optionalIntParam.has_value());
    EXPECT_TRUE(cfg.command->paramList.empty());
    EXPECT_EQ(cfg.command->optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.command->flag, false);
    EXPECT_EQ(cfg.command->arg, 0.f);
    EXPECT_TRUE(cfg.command->argList.empty());
    ASSERT_TRUE(cfg.command->nested.has_value());
    EXPECT_EQ(cfg.command->nested->param, "FOO");
}

struct FullConfigWithOptionalArgList : public Config{
    CMDLIME_PARAM(requiredParam, std::string) << cmdlime::ShortName("r");
    CMDLIME_PARAM(optionalParam, std::string)({"defaultValue"}) << cmdlime::ShortName("o");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i");
    CMDLIME_FLAG(flag) << cmdlime::ShortName("f");
    CMDLIME_ARG(arg, double);
    CMDLIME_ARGLIST(argList, float)({1.f, 2.f});
};

TEST(GNUConfig, MissingOptionalArgList)
{
    auto cfg = FullConfigWithOptionalArgList{};
    cfg.readCommandLine({"-r", "FOO", "4.2"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.f, 2.f}));
}

TEST(GNUConfig, MissingParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-o", "FOO","-L","zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--required-param' is missing."});
        });
}

TEST(GNUConfig, MissingArg)
{
    auto cfg = FullConfigWithOptionalArgList{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r", "FOO", "-o","BAR", "-i","9", "-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'arg' is missing."});
        });
}

TEST(GNUConfig, MissingArgList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r", "FOO", "-o","BAR", "-i","9","-L","zero","-f", "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'arg-list' is missing."});
        });
}

TEST(GNUConfig, MissingParamList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r","FOO", "-o","BAR", "-i","9","-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--param-list' is missing."});
        });
}

TEST(GNUConfig, UnexpectedParam)
{
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r","FOO", "--test","TEST","-L","zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '--test'"});
        });
    }
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r","FOO", "-t","TEST","-L","zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-t'"});
        });
    }
}

TEST(GNUConfig, UnexpectedFlag)
{
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r", "FOO", "--test", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '--test'"});
        });
    }
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r", "FOO", "-t", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-t'"});
        });
    }
}

TEST(GNUConfig, UnexpectedArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::ShortName("p");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-p","FOO", "4.2", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
        });
}

TEST(GNUConfig, WrongParamType)
{
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r", "FOO", "-o","BAR", "-i","nine", "-L","zero", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-int-param' value from 'nine'"});
        });
    }
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r", "FOO", "-o","BAR", "--optional-int-param","nine", "-L","zero", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-int-param' value from 'nine'"});
        });
    }
}

TEST(GNUConfig, WrongParamListElementType)
{
    {
        auto cfg = FullConfig{};
        assert_exception<cmdlime::ParsingError>(
                    [&cfg]{cfg.readCommandLine({"-r","FOO", "-o", "BAR", "-L", "zero", "-O", "not-int", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-param-list' value from 'not-int'"});
        });
    }
    {
        auto cfg = FullConfig{};
        assert_exception<cmdlime::ParsingError>(
                    [&cfg]{cfg.readCommandLine({"-r","FOO", "-o", "BAR", "-L", "zero", "--optional-param-list=not-int", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-param-list' value from 'not-int'"});
        });
    }
}

TEST(GNUConfig, WrongArgType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r", "FOO", "-o", "BAR", "-i","9","-L","zero", "-f", "fortytwo", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'arg' value from 'fortytwo'"});
        });
}

TEST(GNUConfig, WrongArgListElementType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r", "FOO", "-o","BAR", "-i","9", "-L", "zero", "-f", "4.2", "1.1", "2.2", "three"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument list 'arg-list' element's value from 'three'"});
        });
}

TEST(GNUConfig, ParamEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::ShortName("p");
        CMDLIME_FLAG(flag) << cmdlime::ShortName("f");
    };
    {
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
    }
    {
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"--param"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--param' value can't be empty"});
        });
    }
}

TEST(GNUConfig, ParamListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAMLIST(params, std::string) << cmdlime::ShortName("p");
    };
    {
        auto cfg = Cfg{};
        assert_exception<cmdlime::ParsingError>(
                    [&cfg]{cfg.readCommandLine({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
    }
    {
        auto cfg = Cfg{};
        assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"--params"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--params' value can't be empty"});
            });
    }
}

TEST(GNUConfig, ArgEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARG(arg, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg 'arg' value can't be empty"});
        });
}

TEST(GNUConfig, ArgListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARGLIST(args, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"foo", ""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg list 'args' element value can't be empty"});
        });
}


TEST(GNUConfig, ValuesWithWhitespace)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::ShortName("p");
        CMDLIME_PARAMLIST(paramList, std::string) << cmdlime::ShortName("L");
        CMDLIME_ARG(arg, std::string);
        CMDLIME_ARGLIST(argList, std::string);
    };
    {
        auto cfg = Cfg{};
        cfg.readCommandLine({"-p", "Hello world", "-L", "foo bar", "foo bar", "forty two"});
        EXPECT_EQ(cfg.param, "Hello world");
        EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"foo bar"}));
        EXPECT_EQ(cfg.arg, "foo bar");
        EXPECT_EQ(cfg.argList, (std::vector<std::string>{"forty two"}));
    }
    {
        auto cfg = Cfg{};
        cfg.readCommandLine({"--param=Hello world", "--param-list", "foo bar", "foo bar", "forty two"});
        EXPECT_EQ(cfg.param, "Hello world");
        EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"foo bar"}));
        EXPECT_EQ(cfg.arg, "foo bar");
        EXPECT_EQ(cfg.argList, (std::vector<std::string>{"forty two"}));
    }
}

TEST(GNUConfig, ParamWrongNameNonAlphaFirstChar)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::Name("!param");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.readCommandLine({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name '!param' must start with an alphabet character"});
        });
}

TEST(GNUConfig, ParamWrongNameNonAlphanum)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::Name("p$r$m");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.readCommandLine({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'p$r$m' must consist of alphanumeric characters and hyphens"});
        });
}

TEST(GNUConfig, ParamWrongShortNameTooLong)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::ShortName("prm");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.readCommandLine({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's short name 'prm' can't have more than one symbol"});
        });
}

TEST(GNUConfig, ParamWrongShortNameNonAlphanum)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::ShortName("$");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.readCommandLine({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's short name '$' must be an alphanumeric character"});
        });
}

TEST(GNUConfig, NegativeNumberToArg)
{
    struct Cfg : public Config{
        CMDLIME_ARG(arg, int);
        CMDLIME_ARG(argStr, std::string);
        CMDLIME_ARGLIST(argList, double);
    };
    auto cfg = Cfg{};
    cfg.readCommandLine({"-2", "-3", "4.5", "-6.7"});
    EXPECT_EQ(cfg.arg, -2);
    EXPECT_EQ(cfg.argStr, "-3");
    EXPECT_EQ(cfg.argList, (std::vector<double>{4.5, -6.7}));
}

TEST(GNUConfig, NegativeNumberWithoutArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, int) << cmdlime::ShortName("p");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '-2'"});
        });
}

TEST(GNUConfig, ArgsDelimiter)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, int) << cmdlime::ShortName("p");
        CMDLIME_PARAM(optionalParam, int)(0) << cmdlime::ShortName("o");
        CMDLIME_ARGLIST(argList, std::string);
    };

    {
        auto cfg = Cfg{};
        cfg.readCommandLine({"-p", "11", "0", "--", "1", "-o", "2"});
        EXPECT_EQ(cfg.param, 11);
        EXPECT_EQ(cfg.optionalParam, 0);
        EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-o", "2"}));
    }
    {
        auto cfg = Cfg{};
        cfg.readCommandLine({"--param", "11", "0", "--", "1", "-o", "2"});
        EXPECT_EQ(cfg.param, 11);
        EXPECT_EQ(cfg.optionalParam, 0);
        EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-o", "2"}));
    }
}

TEST(GNUConfig, ArgsDelimiterFront)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0) << cmdlime::ShortName("o");
        CMDLIME_ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.readCommandLine({"--", "0", "1", "-o", "1", "2"});
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-o", "1", "2"}));
}

TEST(GNUConfig, ArgsDelimiterBack)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0) << cmdlime::ShortName("o");
        CMDLIME_ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.readCommandLine({"-o","1", "0", "1", "2", "--"});
    EXPECT_EQ(cfg.optionalParam, 1);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "2"}));
}

TEST(GNUConfig, PascalNames)
{
    struct PascalConfig : public Config{
        CMDLIME_PARAM(RequiredParam, std::string);
        CMDLIME_PARAM(OptionalParam, std::string)("defaultValue");
        CMDLIME_PARAM(IntParamOptional, std::optional<int>)();
        CMDLIME_PARAMLIST(ListOfParam, std::string);
        CMDLIME_PARAMLIST(MyListOfParamOptional, int)(std::vector<int>{99, 100});
        CMDLIME_FLAG(Flag);
        CMDLIME_ARG(Arg, double);
        CMDLIME_ARGLIST(ArgList, float);
    };
    auto cfg = PascalConfig{};
    cfg.readCommandLine({"--required-param","FOO", "--optional-param", "BAR", "--int-param-optional","9", "--list-of-param", "zero", "--list-of-param","one",
              "--my-list-of-param-optional","1", "--my-list-of-param-optional", "2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.RequiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.OptionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.IntParamOptional, 9);
    EXPECT_EQ(cfg.ListOfParam, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.MyListOfParamOptional, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.Flag, true);
    EXPECT_EQ(cfg.Arg, 4.2);
    EXPECT_EQ(cfg.ArgList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(GNUConfig, CustomNamesWithoutShortName)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(requiredParam, std::string) << cmdlime::WithoutShortName{};
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue") << cmdlime::WithoutShortName{};
        CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::WithoutShortName{};
        CMDLIME_PARAMLIST(paramList, std::string) << cmdlime::WithoutShortName{};
        CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100}) << cmdlime::WithoutShortName{};
        CMDLIME_FLAG(flag) << cmdlime::WithoutShortName{};
        CMDLIME_FLAG(secondFlag);
        CMDLIME_ARG(arg, double);
        CMDLIME_ARGLIST(argList, float);
    };

    {
    auto cfg = TestConfig{};
    cfg.readCommandLine({"--required-param","FOO", "--optional-param", "BAR", "--optional-int-param","9", "--param-list", "zero", "--param-list","one",
              "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flag, true);
    EXPECT_EQ(cfg.secondFlag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    }

    {
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-r"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-r'"});
        });
    }
    {
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-o"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-o'"});
        });
    }
    {
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-f'"});
        });
    }
    {
    auto cfg = TestConfig{};
    cfg.setProgramName("testproc");
    auto expectedDetailedInfo = std::string{
    "Usage: testproc <arg> --required-param <string> --param-list <string>... [params] [flags] <arg-list...>\n"
    "Arguments:\n"
    "    <arg> (double)                    \n"
    "    <arg-list> (float)                multi-value\n"
    "Parameters:\n"
    "       --required-param <string>      \n"
    "       --param-list <string>          multi-value\n"
    "       --optional-param <string>      optional, default: defaultValue\n"
    "       --optional-int-param <int>     optional\n"
    "       --optional-param-list <int>    multi-value, optional, default: {99, \n"
    "                                        100}\n"
    "Flags:\n"
    "       --flag                         \n"
    "   -s, --second-flag                  \n"};
    EXPECT_EQ(cfg.usageInfoDetailed(), expectedDetailedInfo);
    }
}

TEST(GNUConfig, CustomNamesMissingParam)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(param, double) << cmdlime::Name{"P"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--P' is missing."});
        });
}

TEST(GNUConfig, CustomNamesMissingParamList)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(param, double) << cmdlime::ShortName{"P"};
        CMDLIME_PARAMLIST(paramList, float) << cmdlime::Name{"L"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"-P1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--L' is missing."});
        });
}

TEST(GNUConfig, CustomNamesMissingArg)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(arg, double) << cmdlime::Name{"Argument"};
        CMDLIME_ARGLIST(argList, float) << cmdlime::Name{"ArgList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'Argument' is missing."});
        });
}

TEST(GNUConfig, CustomNamesMissingArgList)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(arg, double) << cmdlime::Name{"Argument"};
        CMDLIME_ARGLIST(argList, float) << cmdlime::Name{"ArgList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.readCommandLine({"1.0"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'ArgList' is missing."});
        });
}

TEST(GNUConfig, ConfigErrorRepeatingParamNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(Param, double)();
        CMDLIME_PARAM(param, int)();
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.readCommandLine({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'param' is already used."});
        });
}

TEST(GNUConfig, ConfigErrorRepeatingParamShortNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(param, double)();
        CMDLIME_PARAMLIST(paramList, int)();
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.readCommandLine({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's short name 'p' is already used."});
        });
}


TEST(GNUConfig, UsageInfo)
{
    auto cfg = FullConfig{};
    cfg.setProgramName("testproc");
    auto expectedInfo = std::string{
    "Usage: testproc [commands] <arg> --required-param <string> --param-list <string>... "
    "[--optional-param <string>] [--optional-int-param <int>] [--optional-param-list <int>...] [--flag] [--second-flag] <arg-list...>\n"
    };
    EXPECT_EQ(cfg.usageInfo(), expectedInfo);
}

TEST(GNUConfig, UsageInfoWithoutMacro)
{
    auto cfg = FullConfigWithoutMacro{};
    cfg.setProgramName("testproc");
    auto expectedInfo = std::string{
            "Usage: testproc [commands] <a> --required-param <string> --prm-list <string>... "
            "[--optional-param <string>] [--optional-int-param <int>] [--optional-prm-list <int>...] [--flg] <a-list...>\n"
    };
    EXPECT_EQ(cfg.usageInfo(), expectedInfo);
}

TEST(GNUConfig, DetailedUsageInfo)
{
    auto cfg = FullConfig{};
    cfg.setProgramName("testproc");
    auto expectedDetailedInfo = std::string{
    "Usage: testproc [commands] <arg> --required-param <string> --param-list <string>... [params] [flags] <arg-list...>\n"
    "Arguments:\n"
    "    <arg> (double)                    \n"
    "    <arg-list> (float)                multi-value\n"
    "Parameters:\n"
    "   -r, --required-param <string>      \n"
    "   -L, --param-list <string>          multi-value\n"
    "   -o, --optional-param <string>      optional, default: defaultValue\n"
    "   -i, --optional-int-param <int>     optional\n"
    "   -O, --optional-param-list <int>    multi-value, optional, default: {99, \n"
    "                                        100}\n"
    "Flags:\n"
    "   -f, --flag                         \n"
    "       --second-flag                  \n"
    "Commands:\n"
    "    command [options]                 \n"
    "    subcommand [options]              \n"};
    EXPECT_EQ(cfg.usageInfoDetailed(), expectedDetailedInfo);
}

TEST(GNUConfig, DetailedUsageInfoWithoutMacro)
{
    auto cfg = FullConfigWithoutMacro{};
    cfg.setProgramName("testproc");
    auto expectedDetailedInfo = std::string{
            "Usage: testproc [commands] <a> --required-param <string> --prm-list <string>... [params] [flags] <a-list...>\n"
            "Arguments:\n"
            "    <a> (double)                      \n"
            "    <a-list> (float)                  multi-value\n"
            "Parameters:\n"
            "   -r, --required-param <string>      \n"
            "   -L, --prm-list <string>            multi-value\n"
            "   -o, --optional-param <string>      optional, default: defaultValue\n"
            "   -i, --optional-int-param <int>     optional\n"
            "   -O, --optional-prm-list <int>      multi-value, optional, default: {99, \n"
            "                                        100}\n"
            "Flags:\n"
            "   -f, --flg                          \n"
            "Commands:\n"
            "    cmd [options]                     \n"
            "    subcommand [options]              \n"};
    EXPECT_EQ(cfg.usageInfoDetailed(), expectedDetailedInfo);
}


TEST(GNUConfig, WrongParamsWithExitFlag){
    struct ConfigWithExitFlag : public Config{
        CMDLIME_PARAM(requiredParam, int);
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
        CMDLIME_FLAG(flag);
        CMDLIME_EXITFLAG(exitFlag);
        CMDLIME_ARG(arg, double);
        CMDLIME_ARGLIST(argList, float);
    } cfg;

    cfg.readCommandLine({"-asd", "asf", "--exit-flag"});
    EXPECT_EQ(cfg.exitFlag, true);
}

struct CustomType{
    std::string value;
};

struct CustomTypeConfig: public Config{
    CMDLIME_PARAM(param, CustomType);
    CMDLIME_PARAMLIST(paramList, CustomType) << cmdlime::ShortName{"l"};
    CMDLIME_ARG(arg, CustomType);
    CMDLIME_ARGLIST(argList, CustomType);
};

struct CustomTypeInt{
    int value;
};

struct CustomTypeIntConfig: public Config{
    CMDLIME_PARAM(param, CustomTypeInt);
    CMDLIME_PARAMLIST(paramList, CustomTypeInt) << cmdlime::ShortName{"l"};
    CMDLIME_ARG(arg, CustomTypeInt);
    CMDLIME_ARGLIST(argList, CustomTypeInt);
};


TEST(GNUConfig, CustomTypeUsage){
    auto cfg = CustomTypeConfig{};
    cfg.readCommandLine({"--param", "hello world", "test arg", "--param-list", "foo bar", "--param-list", "baz", "1", "2 3"});
    EXPECT_EQ(cfg.param.value, "hello world");
    ASSERT_EQ(cfg.paramList.size(), 2); //(std::vector<CustomType>{{"foo bar"}, {"baz"}}));
    EXPECT_EQ(cfg.paramList.at(0).value, "foo bar");
    EXPECT_EQ(cfg.paramList.at(1).value, "baz");
    EXPECT_EQ(cfg.arg.value, "test arg");
    EXPECT_EQ(cfg.argList.size(), 2);
    EXPECT_EQ(cfg.argList.at(0).value, "1");
    EXPECT_EQ(cfg.argList.at(1).value, "2 3");
}

TEST(GNUConfig, CustomTypeIntUsage){
    auto cfg = CustomTypeIntConfig{};
    cfg.readCommandLine({"--param", "10", "42", "--param-list", "0", "--param-list", "1", "43", "44"});
    EXPECT_EQ(cfg.param.value, 10);
    ASSERT_EQ(cfg.paramList.size(), 2); //(std::vector<CustomType>{{"foo bar"}, {"baz"}}));
    EXPECT_EQ(cfg.paramList.at(0).value, 0);
    EXPECT_EQ(cfg.paramList.at(1).value, 1);
    EXPECT_EQ(cfg.arg.value, 42);
    EXPECT_EQ(cfg.argList.size(), 2);
    EXPECT_EQ(cfg.argList.at(0).value, 43);
    EXPECT_EQ(cfg.argList.at(1).value, 44);
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