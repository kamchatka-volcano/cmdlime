#include <gtest/gtest.h>
#include <cmdlime/simpleconfig.h>
#include <cmdlime/configreader.h>
#include "assert_exception.h"
#include <optional>

using Config = cmdlime::SimpleConfig;

struct NestedSubcommandConfig: public Config{
    CMDLIME_PARAM(param, std::string);
};

struct SubcommandConfig: public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_PARAMLIST(paramList, std::string);
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100});
    CMDLIME_FLAG(flag);
    CMDLIME_ARG(arg, double);
    CMDLIME_ARGLIST(argList, float);
    CMDLIME_COMMAND(nested, NestedSubcommandConfig);
};

struct FullConfig : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_PARAMLIST(paramList, std::string);
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100});
    CMDLIME_FLAG(flag);
    CMDLIME_ARG(arg, double);
    CMDLIME_ARGLIST(argList, float);
    CMDLIME_SUBCOMMAND(subcommand, SubcommandConfig);
};

struct FullConfigWithCommand : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_PARAMLIST(paramList, std::string);
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100});
    CMDLIME_FLAG(flag);
    CMDLIME_ARG(arg, double);
    CMDLIME_ARGLIST(argList, float);
    CMDLIME_COMMAND(subcommand, SubcommandConfig);
};


TEST(SimpleConfig, AllSet)
{
    auto cfg = FullConfig{};
    cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
              "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
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

TEST(SimpleConfig, AllSetInSubCommand)
{
    auto cfg = FullConfig{};
    cfg.read({"-requiredParam=FOO", "-paramList=zero", "-paramList=one", "4.2", "1.1",
              "subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
              "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
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

TEST(SimpleConfig, MissingOptionals)
{
    auto cfg = FullConfig{};
    cfg.read({"-requiredParam=FOO", "-paramList=zero", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(SimpleConfig, MissingParamAllSetInSubCommand)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
                         "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-requiredParam' is missing."});
        });
}

TEST(SimpleConfig, AllSetInCommand)
{
    auto cfg = FullConfigWithCommand{};
    cfg.read({"-requiredParam=FOO", "-paramList=zero", "-paramList=one", "4.2", "1.1",
              "subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
              "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
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

TEST(SimpleConfig, MissingParamAllSetInCommand)
{
    auto cfg = FullConfigWithCommand{};
    cfg.read({"subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
              "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
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

TEST(SimpleConfig, MissingParamAllSetInNestedCommand)
{
    auto cfg = FullConfigWithCommand{};
    cfg.read({"subcommand", "nested", "-param=FOO"});
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
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)({"defaultValue"});
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_FLAG(flag);
    CMDLIME_ARG(arg, double);
    CMDLIME_ARGLIST(argList, float)({1.f, 2.f});
};

TEST(SimpleConfig, MissingOptionalArgList)
{
    auto cfg = FullConfigWithOptionalArgList{};
    cfg.read({"-requiredParam=FOO", "4.2"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.f, 2.f}));
}

TEST(SimpleConfig, MissingParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-optionalParam=FOO","-paramList=zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-requiredParam' is missing."});
        });
}

TEST(SimpleConfig, MissingArg)
{
    auto cfg = FullConfigWithOptionalArgList{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flag"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'arg' is missing."});
        });
}

TEST(SimpleConfig, MissingArgList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9","-paramList=zero","--flag", "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'argList' is missing."});
        });
}

TEST(SimpleConfig, MissingParamList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9","--flag", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-paramList' is missing."});
        });
}

TEST(SimpleConfig, UnexpectedParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-testParam=TEST","-paramList=zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter '-testParam'"});
        });
}

TEST(SimpleConfig, UnexpectedFlag)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "--testFlag", "-paramList=zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown flag '--testFlag'"});
        });
}

TEST(SimpleConfig, UnexpectedArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-param=FOO", "4.2", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
        });

}

TEST(SimpleConfig, WrongParamType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=nine", "-paramList=zero", "--flag", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-optionalIntParam' value from 'nine'"});
        });
}

TEST(SimpleConfig, WrongParamListElementType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-paramList=zero", "-optionalParamList=not-int", "--flag", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-optionalParamList' value from 'not-int'"});
        });
}

TEST(SimpleConfig, WrongArgType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9","-paramList=zero", "--flag", "fortytwo", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'arg' value from 'fortytwo'"});
        });
}

TEST(SimpleConfig, WrongArgListElementType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "--flag", "4.2", "1.1", "2.2", "three"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument list 'argList' element's value from 'three'"});
        });
}

TEST(SimpleConfig, ParamWrongNameNonAlphaFirstChar)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::Name("!param");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.read({"-!param=Foo"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name '!param' must start with an alphabet character"});
        });
}

TEST(SimpleConfig, ParamWrongNameNonAlphanum)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::Name("p$r$m");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.read({"-pa-ram=Foo"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'p$r$m' must consist of alphanumeric characters"});
        });
}

TEST(SimpleConfig, MultipleArgLists)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string);
        CMDLIME_ARGLIST(arglist, std::string);
        CMDLIME_ARGLIST(arglist2, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.read({"-param=Foo"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Config can have only one arguments list"});
        });
}

TEST(SimpleConfig, ParamEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-param="});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-param' value can't be empty"});
        });
}

TEST(SimpleConfig, ParamListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAMLIST(params, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-params=", ""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-params' value can't be empty"});
        });
}

TEST(SimpleConfig, ArgEmptyValue)
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

TEST(SimpleConfig, ArgListEmptyValue)
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


TEST(SimpleConfig, ValuesWithWhitespace)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string);
        CMDLIME_PARAMLIST(paramList, std::string);
        CMDLIME_ARG(arg, std::string);
        CMDLIME_ARGLIST(argList, std::string);
    };
    auto cfg = Cfg{};
    cfg.read({"-param=Hello world", "-paramList=foo bar", "foo bar", "forty two"});
    EXPECT_EQ(cfg.param, "Hello world");
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"foo bar"}));
    EXPECT_EQ(cfg.arg, "foo bar");
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"forty two"}));
}

TEST(SimpleConfig, ParamWrongFormat)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-param:2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Wrong parameter format: -param:2. Parameter must have a form of -name=value"});
        });
}

TEST(SimpleConfig, NegativeNumberToArg)
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

TEST(SimpleConfig, NegativeNumberWithoutArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, int);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '-2'"});
        });
}

TEST(SimpleConfig, ArgsDelimiter)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, int);
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.read({"0", "-param=11", "--", "1", "-optionalParam", "2"});
    EXPECT_EQ(cfg.param, 11);
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-optionalParam", "2"}));
}

TEST(SimpleConfig, ArgsDelimiterFront)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.read({"--", "0", "1", "-optionalParam=1", "2"});
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-optionalParam=1", "2"}));
}

TEST(SimpleConfig, ArgsDelimiterBack)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.read({"0", "1", "-optionalParam=1", "2", "--"});
    EXPECT_EQ(cfg.optionalParam, 1);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "2"}));
}

TEST(SimpleConfig, PascalNames)
{
    struct PascalConfig : public Config{
        CMDLIME_PARAM(RequiredParam, std::string);
        CMDLIME_PARAM(OptionalParam, std::string)("defaultValue");
        CMDLIME_PARAM(OptionalIntParam, std::optional<int>)();
        CMDLIME_PARAMLIST(ParamList, std::string);
        CMDLIME_PARAMLIST(OptionalParamList, int)(std::vector<int>{99, 100});
        CMDLIME_FLAG(Flag);
        CMDLIME_ARG(Arg, double);
        CMDLIME_ARGLIST(ArgList, float);
    };
    auto cfg = PascalConfig{};
    cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
              "-optionalParamList=1", "-optionalParamList=2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.RequiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.OptionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.OptionalIntParam, 9);
    EXPECT_EQ(cfg.ParamList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.OptionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.Flag, true);
    EXPECT_EQ(cfg.Arg, 4.2);
    EXPECT_EQ(cfg.ArgList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(SimpleConfig, SnakeNames)
{
    struct PascalConfig : public Config{
        CMDLIME_PARAM(required_param, std::string);
        CMDLIME_PARAM(optional_param, std::string)("defaultValue");
        CMDLIME_PARAM(optional_int_param, std::optional<int>)();
        CMDLIME_PARAMLIST(param_list, std::string);
        CMDLIME_PARAMLIST(optional_param_list_, int)(std::vector<int>{99, 100});
        CMDLIME_FLAG(flag_);
        CMDLIME_ARG(arg_, double);
        CMDLIME_ARGLIST(arg_list_, float);
    };
    auto cfg = PascalConfig{};
    cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
              "-optionalParamList=1", "-optionalParamList=2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.required_param, std::string{"FOO"});
    EXPECT_EQ(cfg.optional_param, std::string{"BAR"});
    EXPECT_EQ(cfg.optional_int_param, 9);
    EXPECT_EQ(cfg.param_list, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optional_param_list_, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flag_, true);
    EXPECT_EQ(cfg.arg_, 4.2);
    EXPECT_EQ(cfg.arg_list_, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(SimpleConfig, CustomNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(requiredParam, std::string) << cmdlime::Name{"customRequiredParam"};
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue") << cmdlime::Name{"customOptionalParam"};
        CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::Name{"customOptionalIntParam"};
        CMDLIME_FLAG(flag) << cmdlime::Name{"customFlag"};
        CMDLIME_ARG(arg, double) << cmdlime::Name{"customArg"};
        CMDLIME_ARGLIST(argList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfg = TestConfig{};
    cfg.read({"-customRequiredParam=FOO", "-customOptionalParam=BAR", "-customOptionalIntParam=9", "--customFlag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.flag, true);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(SimpleConfig, CustomNamesMissingParam)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(param, double) << cmdlime::Name{"customParam"};
        CMDLIME_PARAMLIST(paramList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-customParam' is missing."});
        });
}

TEST(SimpleConfig, CustomNamesMissingParamList)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(param, double) << cmdlime::Name{"customParam"};
        CMDLIME_PARAMLIST(paramList, float) << cmdlime::Name{"customParamList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-customParam=1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-customParamList' is missing."});
        });
}

TEST(SimpleConfig, CustomNamesMissingArg)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(arg, double) << cmdlime::Name{"customArg"};
        CMDLIME_ARGLIST(argList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'customArg' is missing."});
        });
}

TEST(SimpleConfig, CustomNamesMissingArgList)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(arg, double) << cmdlime::Name{"customArg"};
        CMDLIME_ARGLIST(argList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"1.0"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'customArgList' is missing."});
        });
}

TEST(SimpleConfig, ConfigErrorRepeatingParamNames)
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

TEST(SimpleConfig, ConfigErrorRepeatingFlagNames)
{
    struct TestConfig : public Config{
        CMDLIME_FLAG(Flag);
        CMDLIME_FLAG(flag);
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.read({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Flag's name 'flag' is already used."});
        });
}

TEST(SimpleConfig, UsageInfo)
{
    auto cfg = FullConfig{};
    auto expectedInfo = std::string{
    "Usage: testproc [commands] <arg> -requiredParam=<string> -paramList=<string>... "
    "[-optionalParam=<string>] [-optionalIntParam=<int>] [-optionalParamList=<int>...] [--flag] <argList...>\n"
    };
    EXPECT_EQ(cfg.usageInfo("testproc"), expectedInfo);
}

TEST(SimpleConfig, DetailedUsageInfo)
{
    auto cfg = FullConfig{};
    auto expectedDetailedInfo = std::string{
    "Usage: testproc [commands] <arg> -requiredParam=<string> -paramList=<string>... [params] [flags] <argList...>\n"
    "Arguments:\n"
    "    <arg> (double)             \n"
    "    <argList> (float)          multi-value\n"
    "Parameters:\n"
    "   -requiredParam=<string>     \n"
    "   -paramList=<string>         multi-value\n"
    "   -optionalParam=<string>     optional, default: defaultValue\n"
    "   -optionalIntParam=<int>     optional\n"
    "   -optionalParamList=<int>    multi-value, optional, default: {99, 100}\n"
    "Flags:\n"
    "  --flag                       \n"
    "Commands:\n"
    "    subcommand [options]       \n"
    };
    EXPECT_EQ(cfg.usageInfoDetailed("testproc"), expectedDetailedInfo);
}

TEST(SimpleConfig, DetailedUsageInfoFormat)
{
    auto cfg = FullConfig{};
    auto format = cmdlime::UsageInfoFormat{};
    format.columnsSpacing = 2;
    format.nameIndentation = 0;
    format.terminalWidth = 50;
    auto expectedDetailedInfo = std::string{
    "Usage: testproc [commands] <arg> -requiredParam=<string> -paramList=<string>... [params] [flags] <argList...>\n"
    "Arguments:\n"
    "<arg> (double)            \n"
    "<argList> (float)         multi-value\n"
    "Parameters:\n"
    "-requiredParam=<string>   \n"
    "-paramList=<string>       multi-value\n"
    "-optionalParam=<string>   optional, default: \n"
    "                            defaultValue\n"
    "-optionalIntParam=<int>   optional\n"
    "-optionalParamList=<int>  multi-value, optional,\n"
    "                            default: {99, 100}\n"
    "Flags:\n"
    "--flag                    \n"
    "Commands:\n"
    "subcommand [options]      \n"
    };
    EXPECT_EQ(cfg.usageInfoDetailed("testproc", format), expectedDetailedInfo);
}


TEST(SimpleConfig, CustomValueNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::ValueName{"STRING"};
        CMDLIME_PARAMLIST(paramList, int)() << cmdlime::ValueName{"INTS"};
        CMDLIME_ARG(arg, double) << cmdlime::ValueName{"DOUBLE"};
        CMDLIME_ARGLIST(argList, float) << cmdlime::ValueName{"FLOATS"};
    };

    auto cfg = TestConfig{};
    auto expectedInfo = std::string{
    "Usage: testproc <arg> -param=<STRING> [params] <argList...>\n"
    "Arguments:\n"
    "    <arg> (DOUBLE)         \n"
    "    <argList> (FLOATS)     multi-value\n"
    "Parameters:\n"
    "   -param=<STRING>         \n"
    "   -paramList=<INTS>       multi-value, optional, default: {}\n"
    };
    EXPECT_EQ(cfg.usageInfoDetailed("testproc"), expectedInfo);
}

TEST(SimpleConfig, ConfigReaderMissingVersionInfo)
{
    auto cfg = FullConfig{};
    auto reader = cmdlime::ConfigReader{cfg, "testproc", {}, cmdlime::ErrorOutputMode::STDOUT};
    EXPECT_EQ(reader.read({"--version"}), false);
    EXPECT_EQ(reader.exitCode(), -1);
}

TEST(SimpleConfig, ConfigReaderVersion)
{
    auto cfg = FullConfig{};
    cfg.setVersionInfo("testproc 1.0");
    auto reader = cmdlime::ConfigReader{cfg, "testproc"};
    EXPECT_EQ(reader.read({"--version"}), false);
    EXPECT_EQ(reader.exitCode(), 0);
}

TEST(SimpleConfig, ConfigReaderHelp)
{
    auto cfg = FullConfig{};
    auto reader = cmdlime::ConfigReader{cfg, "testproc"};
    EXPECT_EQ(reader.read({"--help"}), false);
    EXPECT_EQ(reader.exitCode(), 0);
}

TEST(SimpleConfig, ConfigReaderCommandHelp)
{
    auto cfg = FullConfigWithCommand{};
    auto reader = cmdlime::ConfigReader{cfg, "testproc"};
    EXPECT_EQ(reader.read({"subcommand", "--help"}), false);
    EXPECT_EQ(reader.exitCode(), 0);
}

TEST(SimpleConfig, ConfigReaderNestedCommandHelp)
{
    auto cfg = FullConfigWithCommand{};
    auto reader = cmdlime::ConfigReader{cfg, "testproc"};
    EXPECT_EQ(reader.read({"subcommand", "nested", "--help"}), false);
    EXPECT_EQ(reader.exitCode(), 0);
}

TEST(SimpleConfig, ConfigReader)
{
    auto cfg = FullConfig{};
    auto reader = cmdlime::ConfigReader{cfg, "testproc"};
    EXPECT_EQ(reader.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
            "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"}), true);
    EXPECT_EQ(reader.exitCode(), 0);
}

TEST(SimpleConfig, WrongParamsWithExitFlag){
    struct ConfigWithExitFlag : public Config{
        CMDLIME_PARAM(requiredParam, int);
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
        CMDLIME_FLAG(flag);
        CMDLIME_EXITFLAG(exitFlag);
        CMDLIME_ARG(arg, double);
        CMDLIME_ARGLIST(argList, float);
    } cfg;

    cfg.read({"asd", "asf", "--exitFlag"});
    EXPECT_EQ(cfg.exitFlag, true);
}



