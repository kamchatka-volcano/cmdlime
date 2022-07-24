#include <gtest/gtest.h>
#include <cmdlime/config.h>
#include <cmdlime/configreader.h>
#include "assert_exception.h"
#include <optional>

using Config = cmdlime::Config;

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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfig>(
            {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=-9", "-paramList=zero", "-paramList=one",
             "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, -9);
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flag, true);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    EXPECT_FALSE(cfg.subcommand);
}

TEST(SimpleConfig, AllSetInSubCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfig>({"-requiredParam=FOO", "-paramList=zero", "-paramList=one", "4.2", "1.1",
                                           "subcommand", "-requiredParam=FOO", "-optionalParam=BAR",
                                           "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
                                           "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f}));
    ASSERT_TRUE(cfg.subcommand);
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfig>({"-requiredParam=FOO", "-paramList=zero", "4.2", "1.1", "2.2", "3.3"});
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{
            cfgReader.read<FullConfig>(
                    {"subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero",
                     "-paramList=one",
                     "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-requiredParam' is missing."});
        });
}

TEST(SimpleConfig, AllSetInCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfigWithCommand>(
            {"-requiredParam=FOO", "-paramList=zero", "-paramList=one", "4.2", "1.1",
             "subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero",
             "-paramList=one",
             "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.paramList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 0.f);
    EXPECT_TRUE(cfg.argList.empty());
    ASSERT_TRUE(cfg.subcommand);
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfigWithCommand>(
            {"subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero",
             "-paramList=one",
             "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.paramList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 0.f);
    EXPECT_TRUE(cfg.argList.empty());
    ASSERT_TRUE(cfg.subcommand);
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfigWithCommand>({"subcommand", "nested", "-param=FOO"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.paramList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 0.f);
    EXPECT_TRUE(cfg.argList.empty());
    ASSERT_TRUE(cfg.subcommand);
    EXPECT_TRUE(cfg.subcommand->requiredParam.empty());
    EXPECT_EQ(cfg.subcommand->optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.subcommand->optionalIntParam.has_value());
    EXPECT_TRUE(cfg.subcommand->paramList.empty());
    EXPECT_EQ(cfg.subcommand->optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.subcommand->flag, false);
    EXPECT_EQ(cfg.subcommand->arg, 0.f);
    EXPECT_TRUE(cfg.subcommand->argList.empty());
    ASSERT_TRUE(cfg.subcommand->nested);
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfigWithOptionalArgList>({"-requiredParam=FOO", "4.2"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.f, 2.f}));
}

TEST(SimpleConfig, MissingParam)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<FullConfig>({"-optionalParam=FOO", "-paramList=zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-requiredParam' is missing."});
        });
}

TEST(SimpleConfig, MissingArg)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{
            cfgReader.read<FullConfigWithOptionalArgList>(
                    {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flag"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'arg' is missing."});
        });
}

TEST(SimpleConfig, MissingArgList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{
            cfgReader.read<FullConfig>(
                    {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "--flag",
                     "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'argList' is missing."});
        });
}

TEST(SimpleConfig, MissingParamList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<FullConfig>(
                {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flag", "4.2", "1.1", "2.2",
                 "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-paramList' is missing."});
        });
}

TEST(SimpleConfig, UnexpectedParam)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{
            cfgReader.read<FullConfig>(
                    {"-requiredParam=FOO", "-testParam=TEST", "-paramList=zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter '-testParam'"});
        });
}

TEST(SimpleConfig, UnexpectedFlag)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{
            cfgReader.read<FullConfig>(
                    {"-requiredParam=FOO", "--testFlag", "-paramList=zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown flag '--testFlag'"});
        });
}

TEST(SimpleConfig, UnexpectedArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<Cfg>({"-param=FOO", "4.2", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
        });

}

TEST(SimpleConfig, WrongParamType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<FullConfig>(
                {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=nine", "-paramList=zero", "--flag",
                 "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-optionalIntParam' value from 'nine'"});
        });
}

TEST(SimpleConfig, WrongParamListElementType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<FullConfig>(
                {"-requiredParam=FOO", "-optionalParam=BAR", "-paramList=zero", "-optionalParamList=not-int", "--flag",
                 "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-optionalParamList' value from 'not-int'"});
        });
}

TEST(SimpleConfig, WrongArgType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<FullConfig>(
                {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "--flag",
                 "fortytwo", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'arg' value from 'fortytwo'"});
        });
}

TEST(SimpleConfig, WrongArgListElementType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<FullConfig>(
                {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "--flag", "4.2",
                 "1.1", "2.2", "three"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument list 'argList' element's value from 'three'"});
        });
}

TEST(SimpleConfig, ParamWrongNameNonAlphaFirstChar)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::Name("!param");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ cfgReader.read<Cfg>({"-!param=Foo"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name '!param' must start with an alphabet character"});
        });
}

TEST(SimpleConfig, ParamWrongNameNonAlphanum)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::Name("p$r$m");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ cfgReader.read<Cfg>({"-pa-ram=Foo"});},
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ cfgReader.read<Cfg>({"-param=Foo"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"BaseConfig can have only one arguments list"});
        });
}

TEST(SimpleConfig, ParamEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<Cfg>({"-param="});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-param' value can't be empty"});
        });
}

TEST(SimpleConfig, ParamListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAMLIST(params, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<Cfg>({"-params=", ""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-params' value can't be empty"});
        });
}

TEST(SimpleConfig, ArgEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARG(arg, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<Cfg>({""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg 'arg' value can't be empty"});
        });
}

TEST(SimpleConfig, ArgListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARGLIST(args, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<Cfg>({"foo", ""});},
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"-param=Hello world", "-paramList=foo bar", "foo bar", "forty two"});
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-param:2"});},
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"-2", "-3", "4.5", "-6.7"});
    EXPECT_EQ(cfg.arg, -2);
    EXPECT_EQ(cfg.argStr, "-3");
    EXPECT_EQ(cfg.argList, (std::vector<double>{4.5, -6.7}));
}

TEST(SimpleConfig, NegativeNumberWithoutArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(param, int);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<Cfg>({"-2"});},
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

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"0", "-param=11", "--", "1", "-optionalParam", "2"});
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

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"--", "0", "1", "-optionalParam=1", "2"});
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-optionalParam=1", "2"}));
}

TEST(SimpleConfig, ArgsDelimiterBack)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argList, std::string);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"0", "1", "-optionalParam=1", "2", "--"});
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<PascalConfig>(
            {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<PascalConfig>(
            {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<TestConfig>(
            {"-customRequiredParam=FOO", "-customOptionalParam=BAR", "-customOptionalIntParam=9", "--customFlag", "4.2",
             "1.1", "2.2", "3.3"});
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({});},
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({"-customParam=1"});},
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({});},
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<TestConfig>({"1.0"});},
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ cfgReader.read<TestConfig>({});},
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
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ cfgReader.read<TestConfig>({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Flag's name 'flag' is already used."});
        });
}

TEST(SimpleConfig, UsageInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    cfgReader.setProgramName("testproc");
    auto expectedInfo = std::string{
    "Usage: testproc [commands] <arg> -requiredParam=<string> -paramList=<string>... "
    "[-optionalParam=<string>] [-optionalIntParam=<int>] [-optionalParamList=<int>...] [--flag] <argList...>\n"
    };
    EXPECT_EQ(cfgReader.usageInfo<FullConfig>(), expectedInfo);
}

TEST(SimpleConfig, DetailedUsageInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    cfgReader.setProgramName("testproc");
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
    EXPECT_EQ(cfgReader.usageInfoDetailed<FullConfig>(), expectedDetailedInfo);
}

TEST(SimpleConfig, DetailedUsageInfoFormat)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    cfgReader.setProgramName("testproc");
    auto format = cmdlime::UsageInfoFormat{};
    format.columnsSpacing = 2;
    format.nameIndentation = 0;
    format.terminalWidth = 50;
    cfgReader.setUsageInfoFormat(format);
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
    EXPECT_EQ(cfgReader.usageInfoDetailed<FullConfig>(), expectedDetailedInfo);
}


TEST(SimpleConfig, CustomValueNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(param, std::string) << cmdlime::ValueName{"STRING"};
        CMDLIME_PARAMLIST(paramList, int)() << cmdlime::ValueName{"INTS"};
        CMDLIME_ARG(arg, double) << cmdlime::ValueName{"DOUBLE"};
        CMDLIME_ARGLIST(argList, float) << cmdlime::ValueName{"FLOATS"};
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    cfgReader.setProgramName("testproc");
    auto expectedInfo = std::string{
    "Usage: testproc <arg> -param=<STRING> [params] <argList...>\n"
    "Arguments:\n"
    "    <arg> (DOUBLE)         \n"
    "    <argList> (FLOATS)     multi-value\n"
    "Parameters:\n"
    "   -param=<STRING>         \n"
    "   -paramList=<INTS>       multi-value, optional, default: {}\n"
    };
    EXPECT_EQ(cfgReader.usageInfoDetailed<TestConfig>(), expectedInfo);
}


TEST(SimpleConfig, WrongParamsWithExitFlag){
    struct ConfigWithExitFlag : public Config{
        CMDLIME_PARAM(requiredParam, int);
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
        CMDLIME_FLAG(flag);
        CMDLIME_EXITFLAG(exitFlag);
        CMDLIME_ARG(arg, double);
        CMDLIME_ARGLIST(argList, float);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<ConfigWithExitFlag>({"asd", "asf", "--exitFlag"});
    EXPECT_EQ(cfg.exitFlag, true);
}

TEST(SimpleConfig, ExecMissingVersionInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    auto errorOutput = std::stringstream{};
    cfgReader.setErrorOutputStream(errorOutput);

    EXPECT_EQ(cfgReader.exec<FullConfig>({"--version"}, [](const FullConfig&){ return 0;}), -1);
    EXPECT_EQ(errorOutput.str(), "Encountered unknown flag '--version'\n");
}

TEST(SimpleConfig, ExecVersion)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    cfgReader.setVersionInfo("testproc 1.0");
    auto output = std::stringstream{};
    cfgReader.setOutputStream(output);
    EXPECT_EQ(cfgReader.exec<FullConfig>({"--version"}, [](const FullConfig&){ return 0;}), 0);
    EXPECT_EQ(output.str(), "testproc 1.0\n");
}

TEST(SimpleConfig, ExecHelp)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    auto output = std::stringstream{};
    cfgReader.setOutputStream(output);
    EXPECT_EQ(cfgReader.exec<FullConfig>({"--help"}, [](const FullConfig&){ return 0;}), 0);
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
            "  --help                       show usage info and exit\n"
            "Commands:\n"
            "    subcommand [options]       \n\n"
    };
    EXPECT_EQ(output.str(), expectedDetailedInfo);
}

TEST(SimpleConfig, ExecHelpUsageInfoFormat)
{
    auto cfg = FullConfig{};
    auto format = cmdlime::UsageInfoFormat{};
    format.columnsSpacing = 2;
    format.nameIndentation = 0;
    format.terminalWidth = 50;
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc", format};
    auto output = std::stringstream{};
    cfgReader.setOutputStream(output);
    EXPECT_EQ(cfgReader.exec<FullConfig>({"--help"}, [](const FullConfig&){ return 0;}), 0);
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
            "--help                    show usage info and \n"
            "                            exit\n"
            "Commands:\n"
            "subcommand [options]      \n\n"
    };
    EXPECT_EQ(output.str(), expectedDetailedInfo);
}

TEST(SimpleConfig, ExecCommandHelp)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    auto output = std::stringstream{};
    cfgReader.setOutputStream(output);
    auto expectedDetailedInfo = std::string{
            "Usage: testproc subcommand [commands] <arg> -requiredParam=<string> -paramList=<string>... [params] [flags] <argList...>\n"
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
            "  --help                       show usage info and exit\n"
            "Commands:\n"
            "    nested [options]           \n\n"
    };
    EXPECT_EQ(cfgReader.exec<FullConfigWithCommand>({"subcommand", "--help"}, [](const auto&){return 0;}), 0);
    EXPECT_EQ(output.str(), expectedDetailedInfo);
}

TEST(SimpleConfig, ExecCommandHelpUsageInfoFormat)
{
    auto format = cmdlime::UsageInfoFormat{};
    format.columnsSpacing = 2;
    format.nameIndentation = 0;
    format.terminalWidth = 50;
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc", format};
    auto output = std::stringstream{};
    cfgReader.setOutputStream(output);
    EXPECT_EQ(cfgReader.exec<FullConfigWithCommand>({"subcommand", "--help"}, [](const auto&){return 0;}), 0);
    auto expectedDetailedInfo = std::string{
            "Usage: testproc subcommand [commands] <arg> -requiredParam=<string> -paramList=<string>... [params] [flags] <argList...>\n"
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
            "--help                    show usage info and \n"
            "                            exit\n"
            "Commands:\n"
            "nested [options]          \n\n"
    };
    EXPECT_EQ(output.str(), expectedDetailedInfo);
}

TEST(SimpleConfig, ExecNestedCommandHelp)
{
    auto cfg = FullConfigWithCommand{};
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    auto output = std::stringstream{};
    cfgReader.setOutputStream(output);
    auto expectedDetailedInfo = std::string{
            "Usage: testproc subcommand nested -param=<string> [flags] \n"
            "Parameters:\n"
            "   -param=<string>     \n"
            "Flags:\n"
            "  --help               show usage info and exit\n\n"
    };
    EXPECT_EQ(cfgReader.exec<FullConfigWithCommand>({"subcommand", "nested", "--help"}, [](const auto&){return 0;}), 0);
    EXPECT_EQ(output.str(), expectedDetailedInfo);
}

TEST(SimpleConfig, ExecNestedCommandHelpUsageInfoFormat)
{
    auto cfg = FullConfigWithCommand{};
    auto format = cmdlime::UsageInfoFormat{};
    format.columnsSpacing = 2;
    format.nameIndentation = 0;
    format.terminalWidth = 50;
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc", format};
    auto output = std::stringstream{};
    cfgReader.setOutputStream(output);
    auto expectedDetailedInfo = std::string{
            "Usage: testproc subcommand nested -param=<string> [flags] \n"
            "Parameters:\n"
            "-param=<string>   \n"
            "Flags:\n"
            "--help            show usage info and exit\n\n"
    };
    EXPECT_EQ(cfgReader.exec<FullConfigWithCommand>({"subcommand", "nested", "--help"}, [](const auto&){return 0;}), 0);
    EXPECT_EQ(output.str(), expectedDetailedInfo);
}

TEST(SimpleConfig, ExecSuccess)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    EXPECT_EQ(cfgReader.exec<FullConfig>({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
            "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"}, [](const auto&){return 0;}), 0);
}

TEST(SimpleConfig, ExecError)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    auto errorOutput = std::stringstream{};
    cfgReader.setErrorOutputStream(errorOutput);
    EXPECT_EQ(cfgReader.exec<FullConfig>({"-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
            "-optionalParamList=1,2", "--flag", "4.2", "1.1", "2.2", "3.3"}, [](const auto&){return 0;}), -1);
    EXPECT_EQ(errorOutput.str(),"Parameter '-requiredParam' is missing.\n");
}
