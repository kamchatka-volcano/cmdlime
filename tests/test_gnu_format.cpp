#include <gtest/gtest.h>
#include <cmdlime/gnuconfig.h>
#include "assert_exception.h"
#include <optional>

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
    CMDLIME_SUBCOMMAND(subcommand, SubcommandConfig);
};

struct FullConfigWithCommand : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i");
    CMDLIME_PARAMLIST(paramList, std::string) << cmdlime::ShortName("L");
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100}) << cmdlime::ShortName("O");
    CMDLIME_FLAG(flag);
    CMDLIME_ARG(arg, double);
    CMDLIME_ARGLIST(argList, float);
    CMDLIME_COMMAND(subcommand, SubcommandConfig);
};

TEST(GNUConfig, AllSet)
{
    auto cfg = FullConfig{};
    cfg.read({"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
              "--optional-param-list=1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flag, true);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    EXPECT_FALSE(cfg.subcommand.has_value());
}

TEST(GNUConfig, AllSetInSubCommand)
{
    auto cfg = FullConfig{};
    cfg.read({"-r", "FOO", "--param-list=zero", "--param-list=one", "4.2", "1.1",
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
    cfg.read({"-fst", "-pfirst"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.param, "first");
    }

    {
    auto cfg = Cfg{};
    cfg.read({"-tfspfirst"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.param, "first");
    }

    {
    auto cfg = Cfg{};
    cfg.read({"-fs","-tp" "first"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.param, "first");
    }

    {
    auto cfg = Cfg{};
    cfg.read({"-fs", "--param", "first"});
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
        cfg.read({"-1", "-2", "test", "-99"});
        EXPECT_EQ(cfg.flag, true);
        EXPECT_EQ(cfg.param, "test");
        EXPECT_EQ(cfg.paramSecond, "default");
        EXPECT_EQ(cfg.arg, -99);
    }
    {
        auto cfg = Cfg{};
        cfg.read({"-2test", "-1", "--param-second=second", "-99"});
        EXPECT_EQ(cfg.flag, true);
        EXPECT_EQ(cfg.param, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.arg, -99);
    }
    {
        auto cfg = Cfg{};
        cfg.read({"-12test", "--param-second", "second", "-99"});
        EXPECT_EQ(cfg.flag, true);
        EXPECT_EQ(cfg.param, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.arg, -99);
    }
}

TEST(GNUConfig, MissingOptionals)
{
    auto cfg = FullConfig{};
    cfg.read({"-r", "FOO", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});
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
        [&cfg]{cfg.read({"subcommand", "--required-param=FOO", "--optional-param=BAR", "--optional-int-param=9", "--param-list","zero", "--param-list=one",
                         "--optional-param-list=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--required-param' is missing."});
        });
}

TEST(GNUConfig, AllSetInCommand)
{
    auto cfg = FullConfigWithCommand{};
    cfg.read({"--required-param=FOO", "--param-list=zero", "--param-list=one", "4.2", "1.1",
              "subcommand", "--required-param=FOO", "--optional-param=BAR", "--optional-int-param=9", "--param-list=zero", "--param-list=one",
              "--optional-param-list=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.paramList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 0.f);
    EXPECT_TRUE(cfg.argList.empty());
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

TEST(GNUConfig, MissingParamAllSetInCommand)
{
    auto cfg = FullConfigWithCommand{};
    cfg.read({"subcommand", "--required-param=FOO", "--optional-param=BAR", "--optional-int-param=9", "--param-list=zero", "--param-list=one",
              "--optional-param-list=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.paramList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 0.f);
    EXPECT_TRUE(cfg.argList.empty());
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

TEST(GNUConfig, MissingParamAllSetInNestedCommand)
{
    auto cfg = FullConfigWithCommand{};
    cfg.read({"subcommand", "nested", "--param=FOO"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.paramList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 0.f);
    EXPECT_TRUE(cfg.argList.empty());
    ASSERT_TRUE(cfg.subcommand.has_value());
    EXPECT_TRUE(cfg.subcommand->requiredParam.empty());
    EXPECT_EQ(cfg.subcommand->optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.subcommand->optionalIntParam.has_value());
    EXPECT_TRUE(cfg.subcommand->paramList.empty());
    EXPECT_EQ(cfg.subcommand->optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.subcommand->flag, false);
    EXPECT_EQ(cfg.subcommand->arg, 0.f);
    EXPECT_TRUE(cfg.subcommand->argList.empty());
    ASSERT_TRUE(cfg.subcommand->nested.has_value());
    EXPECT_EQ(cfg.subcommand->nested->param, "FOO");
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
    cfg.read({"-r", "FOO", "4.2"});
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
        [&cfg]{cfg.read({"-o", "FOO","-L","zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--required-param' is missing."});
        });
}

TEST(GNUConfig, MissingArg)
{
    auto cfg = FullConfigWithOptionalArgList{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o","BAR", "-i","9", "-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'arg' is missing."});
        });
}

TEST(GNUConfig, MissingArgList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o","BAR", "-i","9","-L","zero","-f", "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'arg-list' is missing."});
        });
}

TEST(GNUConfig, MissingParamList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r","FOO", "-o","BAR", "-i","9","-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--param-list' is missing."});
        });
}

TEST(GNUConfig, UnexpectedParam)
{
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r","FOO", "--test","TEST","-L","zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '--test'"});
        });
    }
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r","FOO", "-t","TEST","-L","zero", "4.2", "1.1", "2.2", "3.3"});},
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
        [&cfg]{cfg.read({"-r", "FOO", "--test", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '--test'"});
        });
    }
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-t", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
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
        [&cfg]{cfg.read({"-p","FOO", "4.2", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
        });
}

TEST(GNUConfig, WrongParamType)
{
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o","BAR", "-i","nine", "-L","zero", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-int-param' value from 'nine'"});
        });
    }
    {
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o","BAR", "--optional-int-param","nine", "-L","zero", "-f", "4.2", "1.1", "2.2", "3.3"});},
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
                    [&cfg]{cfg.read({"-r","FOO", "-o", "BAR", "-L", "zero", "-O", "not-int", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-param-list' value from 'not-int'"});
        });
    }
    {
        auto cfg = FullConfig{};
        assert_exception<cmdlime::ParsingError>(
                    [&cfg]{cfg.read({"-r","FOO", "-o", "BAR", "-L", "zero", "--optional-param-list=not-int", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '--optional-param-list' value from 'not-int'"});
        });
    }
}

TEST(GNUConfig, WrongArgType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o", "BAR", "-i","9","-L","zero", "-f", "fortytwo", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'arg' value from 'fortytwo'"});
        });
}

TEST(GNUConfig, WrongArgListElementType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o","BAR", "-i","9", "-L", "zero", "-f", "4.2", "1.1", "2.2", "three"});},
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
        [&cfg]{cfg.read({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
    }
    {
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"--param"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '--param' value can't be empty"});
        });
    }
    {
        auto cfg = Cfg{};
        assert_exception<cmdlime::ParsingError>(
                    [&cfg]{cfg.read({"-p", "-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
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
                    [&cfg]{cfg.read({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
    }
    {
        auto cfg = Cfg{};
        assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.read({"--params"});},
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
        [&cfg]{cfg.read({""});},
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
        [&cfg]{cfg.read({"foo", ""});},
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
        cfg.read({"-p", "Hello world", "-L", "foo bar", "foo bar", "forty two"});
        EXPECT_EQ(cfg.param, "Hello world");
        EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"foo bar"}));
        EXPECT_EQ(cfg.arg, "foo bar");
        EXPECT_EQ(cfg.argList, (std::vector<std::string>{"forty two"}));
    }
    {
        auto cfg = Cfg{};
        cfg.read({"--param=Hello world", "--param-list", "foo bar", "foo bar", "forty two"});
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
        [&cfg]{cfg.read({"-pname"});},
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
        [&cfg]{cfg.read({"-pname"});},
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
        [&cfg]{cfg.read({"-pname"});},
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
        [&cfg]{cfg.read({"-pname"});},
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
    cfg.read({"-2", "-3", "4.5", "-6.7"});
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
        [&cfg]{cfg.read({"-2"});},
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
        cfg.read({"-p", "11", "0", "--", "1", "-o", "2"});
        EXPECT_EQ(cfg.param, 11);
        EXPECT_EQ(cfg.optionalParam, 0);
        EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-o", "2"}));
    }
    {
        auto cfg = Cfg{};
        cfg.read({"--param", "11", "0", "--", "1", "-o", "2"});
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
    cfg.read({"--", "0", "1", "-o", "1", "2"});
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
    cfg.read({"-o","1", "0", "1", "2", "--"});
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
    cfg.read({"--required-param","FOO", "--optional-param", "BAR", "--int-param-optional","9", "--list-of-param", "zero", "--list-of-param","one",
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
    cfg.read({"--required-param","FOO", "--optional-param", "BAR", "--optional-int-param","9", "--param-list", "zero", "--param-list","one",
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
        [&cfg]{cfg.read({"-r"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-r'"});
        });
    }
    {
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-o"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-o'"});
        });
    }
    {
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-f'"});
        });
    }
    {
    auto cfg = TestConfig{};
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
    EXPECT_EQ(cfg.usageInfoDetailed("testproc"), expectedDetailedInfo);
    }
}

TEST(GNUConfig, CustomNamesMissingParam)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(param, double) << cmdlime::Name{"P"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({});},
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
        [&cfg]{cfg.read({"-P1"});},
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
        [&cfg]{cfg.read({});},
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
        [&cfg]{cfg.read({"1.0"});},
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
        [&cfg]{cfg.read({});},
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
        [&cfg]{cfg.read({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's short name 'p' is already used."});
        });
}


TEST(GNUConfig, UsageInfo)
{
    auto cfg = FullConfig{};
    auto expectedInfo = std::string{
    "Usage: testproc [commands] <arg> --required-param <string> --param-list <string>... "
    "[--optional-param <string>] [--optional-int-param <int>] [--optional-param-list <int>...] [--flag] [--second-flag] <arg-list...>\n"
    };
    EXPECT_EQ(cfg.usageInfo("testproc"), expectedInfo);
}

TEST(GNUConfig, DetailedUsageInfo)
{
    auto cfg = FullConfig{};
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
    "    subcommand [options]              \n"};
    EXPECT_EQ(cfg.usageInfoDetailed("testproc"), expectedDetailedInfo);
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

    cfg.read({"-asd", "asf", "--exit-flag"});
    EXPECT_EQ(cfg.exitFlag, true);
}

}
