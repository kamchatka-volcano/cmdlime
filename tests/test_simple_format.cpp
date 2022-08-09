#include <gtest/gtest.h>
#include <cmdlime/config.h>
#include <cmdlime/configreader.h>
#include "assert_exception.h"
#include <optional>

namespace test_simple_format {

using namespace cmdlime;

struct NestedSubcommandConfig : public Config {
    CMDLIME_PARAM(prm, std::string);
};

struct SubcommandConfig : public Config {
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_PARAMLIST(prmList, std::vector<std::string>);
    CMDLIME_PARAMLIST(optionalParamList, std::vector<int>)(std::vector<int>{99, 100});
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, std::vector<float>);
    CMDLIME_COMMAND(nested, NestedSubcommandConfig);
};

struct FullConfig : public Config {
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_PARAMLIST(prmList, std::vector<std::string>);
    CMDLIME_PARAMLIST(optionalParamList, std::vector<int>)(std::vector<int>{99, 100});
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, std::vector<float>);
    CMDLIME_SUBCOMMAND(subcommand, SubcommandConfig);
};

struct FullConfigWithCommand : public Config {
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_PARAMLIST(prmList, std::vector<std::string>);
    CMDLIME_PARAMLIST(optionalParamList, std::vector<int>)(std::vector<int>{99, 100});
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, std::vector<float>);
    CMDLIME_COMMAND(subcommand, SubcommandConfig);
};


TEST(SimpleConfig, AllSet)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfig>(
            {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=-9", "-prmList=zero", "-prmList=one",
             "-optionalParamList=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, -9);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flg, true);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    EXPECT_FALSE(cfg.subcommand);
}

TEST(SimpleConfig, AllSetInSubCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfig>({"-requiredParam=FOO", "-prmList=zero", "-prmList=one", "4.2", "1.1",
                                           "subcommand", "-requiredParam=FOO", "-optionalParam=BAR",
                                           "-optionalIntParam=9", "-prmList=zero", "-prmList=one",
                                           "-optionalParamList=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f}));
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

TEST(SimpleConfig, MissingOptionals)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfig>({"-requiredParam=FOO", "-prmList=zero", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(SimpleConfig, MissingParamAllSetInSubCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfig>(
                        {"subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9",
                         "-prmList=zero",
                         "-prmList=one",
                         "-optionalParamList=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-requiredParam' is missing."});
            });
}

TEST(SimpleConfig, AllSetInCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfigWithCommand>(
            {"-requiredParam=FOO", "-prmList=zero", "-prmList=one", "4.2", "1.1",
             "subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-prmList=zero",
             "-prmList=one",
             "-optionalParamList=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.prmList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 0.f);
    EXPECT_TRUE(cfg.argumentList.empty());
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

TEST(SimpleConfig, MissingParamAllSetInCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfigWithCommand>(
            {"subcommand", "-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-prmList=zero",
             "-prmList=one",
             "-optionalParamList=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.prmList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 0.f);
    EXPECT_TRUE(cfg.argumentList.empty());
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

TEST(SimpleConfig, MissingParamAllSetInNestedCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfigWithCommand>({"subcommand", "nested", "-prm=FOO"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.prmList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 0.f);
    EXPECT_TRUE(cfg.argumentList.empty());
    ASSERT_TRUE(cfg.subcommand);
    EXPECT_TRUE(cfg.subcommand->requiredParam.empty());
    EXPECT_EQ(cfg.subcommand->optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.subcommand->optionalIntParam.has_value());
    EXPECT_TRUE(cfg.subcommand->prmList.empty());
    EXPECT_EQ(cfg.subcommand->optionalParamList, (std::vector<int>{99, 100}));
    EXPECT_EQ(cfg.subcommand->flg, false);
    EXPECT_EQ(cfg.subcommand->argument, 0.f);
    EXPECT_TRUE(cfg.subcommand->argumentList.empty());
    ASSERT_TRUE(cfg.subcommand->nested);
    EXPECT_EQ(cfg.subcommand->nested->prm, "FOO");
}


struct FullConfigWithOptionalArgList : public Config {
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)({"defaultValue"});
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, std::vector<float>)({1.f, 2.f});
};

TEST(SimpleConfig, MissingOptionalArgList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<FullConfigWithOptionalArgList>({"-requiredParam=FOO", "4.2"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.f, 2.f}));
}

TEST(SimpleConfig, MissingParam)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { cfgReader.read<FullConfig>({"-optionalParam=FOO", "-prmList=zero", "4.2", "1.1", "2.2", "3.3"}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-requiredParam' is missing."});
            });
}

TEST(SimpleConfig, MissingArg)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfigWithOptionalArgList>(
                        {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flg"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'argument' is missing."});
            });
}

TEST(SimpleConfig, MissingArgList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfig>(
                        {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-prmList=zero", "--flg",
                         "4.2"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'argumentList' is missing."});
            });
}

TEST(SimpleConfig, MissingParamList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfig>(
                        {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flg", "4.2", "1.1",
                         "2.2",
                         "3.3"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-prmList' is missing."});
            });
}

TEST(SimpleConfig, UnexpectedParam)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfig>(
                        {"-requiredParam=FOO", "-testParam=TEST", "-prmList=zero", "4.2", "1.1", "2.2", "3.3"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter '-testParam'"});
            });
}

TEST(SimpleConfig, UnexpectedFlag)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfig>(
                        {"-requiredParam=FOO", "--testFlag", "-prmList=zero", "4.2", "1.1", "2.2", "3.3"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown flag '--testFlag'"});
            });
}

TEST(SimpleConfig, UnexpectedArg)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(prm, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { cfgReader.read<Cfg>({"-prm=FOO", "4.2", "1"}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
            });

}

TEST(SimpleConfig, WrongParamType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfig>(
                        {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=nine", "-prmList=zero", "--flg",
                         "4.2", "1.1", "2.2", "3.3"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()},
                          std::string{"Couldn't set parameter '-optionalIntParam' value from 'nine'"});
            });
}

TEST(SimpleConfig, WrongParamListElementType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfig>(
                        {"-requiredParam=FOO", "-optionalParam=BAR", "-prmList=zero", "-optionalParamList=not-int",
                         "--flg",
                         "4.2", "1.1", "2.2", "3.3"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()},
                          std::string{"Couldn't set parameter '-optionalParamList' value from 'not-int'"});
            });
}

TEST(SimpleConfig, WrongArgType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfig>(
                        {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-prmList=zero", "--flg",
                         "fortytwo", "1.1", "2.2", "3.3"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()},
                          std::string{"Couldn't set argument 'argument' value from 'fortytwo'"});
            });
}

TEST(SimpleConfig, WrongArgListElementType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] {
                cfgReader.read<FullConfig>(
                        {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-prmList=zero", "--flg",
                         "4.2",
                         "1.1", "2.2", "three"});
            },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()},
                          std::string{"Couldn't set argument list 'argumentList' element's value from 'three'"});
            });
}

TEST(SimpleConfig, ParamWrongNameNonAlphaFirstChar)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(prm, std::string) << cmdlime::Name("!param");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
            [&] { cfgReader.read<Cfg>({"-!param=Foo"}); },
            [](const cmdlime::ConfigError& error) {
                EXPECT_EQ(std::string{error.what()},
                          std::string{"Parameter's name '!param' must start with an alphabet character"});
            });
}

TEST(SimpleConfig, ParamWrongNameNonAlphanum)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(prm, std::string) << cmdlime::Name("p$r$m");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
            [&] { cfgReader.read<Cfg>({"-pa-ram=Foo"}); },
            [](const cmdlime::ConfigError& error) {
                EXPECT_EQ(std::string{error.what()},
                          std::string{"Parameter's name 'p$r$m' must consist of alphanumeric characters"});
            });
}

TEST(SimpleConfig, MultipleArgLists)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(prm, std::string);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>);
        CMDLIME_ARGLIST(argumentList2, std::vector<std::string>);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
            [&] { cfgReader.read<Cfg>({"-param=Foo"}); },
            [](const cmdlime::ConfigError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"BaseConfig can have only one arguments list"});
            });
}

TEST(SimpleConfig, ParamEmptyValue)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(prm, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { cfgReader.read<Cfg>({"-param="}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-param' value can't be empty"});
            });
}

TEST(SimpleConfig, ParamListEmptyValue)
{
    struct Cfg : public Config {
        CMDLIME_PARAMLIST(params, std::vector<std::string>);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { cfgReader.read<Cfg>({"-params=", ""}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-params' value can't be empty"});
            });
}

TEST(SimpleConfig, ArgEmptyValue)
{
    struct Cfg : public Config {
        CMDLIME_ARG(argument, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { cfgReader.read<Cfg>({""}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Arg 'argument' value can't be empty"});
            });
}

TEST(SimpleConfig, ArgListEmptyValue)
{
    struct Cfg : public Config {
        CMDLIME_ARGLIST(args, std::vector<std::string>);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { cfgReader.read<Cfg>({"foo", ""}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Arg list 'args' element value can't be empty"});
            });
}


TEST(SimpleConfig, ValuesWithWhitespace)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(prm, std::string);
        CMDLIME_PARAMLIST(prmList, std::vector<std::string>);
        CMDLIME_ARG(argument, std::string);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"-prm=Hello world", "-prmList=foo bar", "foo bar", "forty two"});
    EXPECT_EQ(cfg.prm, "Hello world");
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"foo bar"}));
    EXPECT_EQ(cfg.argument, "foo bar");
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"forty two"}));
}

TEST(SimpleConfig, ParamWrongFormat)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(prm, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { auto cfg = cfgReader.read<Cfg>({"-param:2"}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()},
                          std::string{"Wrong parameter format: -param:2. Parameter must have a form of -name=value"});
            });
}

TEST(SimpleConfig, NegativeNumberToArg)
{
    struct Cfg : public Config {
        CMDLIME_ARG(argument, int);
        CMDLIME_ARG(argStr, std::string);
        CMDLIME_ARGLIST(argumentList, std::vector<double>);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"-2", "-3", "4.5", "-6.7"});
    EXPECT_EQ(cfg.argument, -2);
    EXPECT_EQ(cfg.argStr, "-3");
    EXPECT_EQ(cfg.argumentList, (std::vector<double>{4.5, -6.7}));
}

TEST(SimpleConfig, NegativeNumberWithoutArg)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(prm, int);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { cfgReader.read<Cfg>({"-2"}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '-2'"});
            });
}

TEST(SimpleConfig, ArgsDelimiter)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(prm, int);
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"0", "-prm=11", "--", "1", "-optionalParam", "2"});
    EXPECT_EQ(cfg.prm, 11);
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "-optionalParam", "2"}));
}

TEST(SimpleConfig, ArgsDelimiterFront)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"--", "0", "1", "-optionalParam=1", "2"});
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "-optionalParam=1", "2"}));
}

TEST(SimpleConfig, ArgsDelimiterBack)
{
    struct Cfg : public Config {
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<Cfg>({"0", "1", "-optionalParam=1", "2", "--"});
    EXPECT_EQ(cfg.optionalParam, 1);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "2"}));
}

TEST(SimpleConfig, PascalNames)
{
    struct PascalConfig : public Config {
        CMDLIME_PARAM(RequiredParam, std::string);
        CMDLIME_PARAM(OptionalParam, std::string)("defaultValue");
        CMDLIME_PARAM(OptionalIntParam, std::optional<int>)();
        CMDLIME_PARAMLIST(prmList, std::vector<std::string>);
        CMDLIME_PARAMLIST(OptionalParamList, std::vector<int>)(std::vector<int>{99, 100});
        CMDLIME_FLAG(flg);
        CMDLIME_ARG(Arg, double);
        CMDLIME_ARGLIST(argumentList, std::vector<float>);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<PascalConfig>(
            {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-prmList=zero", "-prmList=one",
             "-optionalParamList=1", "-optionalParamList=2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.RequiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.OptionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.OptionalIntParam, 9);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.OptionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flg, true);
    EXPECT_EQ(cfg.Arg, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(SimpleConfig, SnakeNames)
{
    struct PascalConfig : public Config {
        CMDLIME_PARAM(required_param, std::string);
        CMDLIME_PARAM(optional_param, std::string)("defaultValue");
        CMDLIME_PARAM(optional_int_param, std::optional<int>)();
        CMDLIME_PARAMLIST(param_list, std::vector<std::string>);
        CMDLIME_PARAMLIST(optional_param_list_, std::vector<int>)(std::vector<int>{99, 100});
        CMDLIME_FLAG(flg_);
        CMDLIME_ARG(arg_, double);
        CMDLIME_ARGLIST(arg_list_, std::vector<float>);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<PascalConfig>(
            {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
             "-optionalParamList=1", "-optionalParamList=2", "--flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.required_param, std::string{"FOO"});
    EXPECT_EQ(cfg.optional_param, std::string{"BAR"});
    EXPECT_EQ(cfg.optional_int_param, 9);
    EXPECT_EQ(cfg.param_list, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optional_param_list_, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flg_, true);
    EXPECT_EQ(cfg.arg_, 4.2);
    EXPECT_EQ(cfg.arg_list_, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(SimpleConfig, CustomNames)
{
    struct TestConfig : public Config {
        CMDLIME_PARAM(requiredParam, std::string) << cmdlime::Name{"customRequiredParam"};
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue") << cmdlime::Name{"customOptionalParam"};
        CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::Name{"customOptionalIntParam"};
        CMDLIME_FLAG(flg) << cmdlime::Name{"customFlag"};
        CMDLIME_ARG(argument, double) << cmdlime::Name{"customArg"};
        CMDLIME_ARGLIST(argumentList, std::vector<float>) << cmdlime::Name{"customArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<TestConfig>(
            {"-customRequiredParam=FOO", "-customOptionalParam=BAR", "-customOptionalIntParam=9", "--customFlag", "4.2",
             "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.flg, true);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(SimpleConfig, CustomNamesMissingParam)
{
    struct TestConfig : public Config {
        CMDLIME_PARAM(prm, double) << cmdlime::Name{"customParam"};
        CMDLIME_PARAMLIST(prmList, std::vector<float>) << cmdlime::Name{"customArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { auto cfg = cfgReader.read<TestConfig>({}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-customParam' is missing."});
            });
}

TEST(SimpleConfig, CustomNamesMissingParamList)
{
    struct TestConfig : public Config {
        CMDLIME_PARAM(prm, double) << cmdlime::Name{"customParam"};
        CMDLIME_PARAMLIST(prmList, std::vector<float>) << cmdlime::Name{"customParamList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { auto cfg = cfgReader.read<TestConfig>({"-customParam=1"}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-customParamList' is missing."});
            });
}

TEST(SimpleConfig, CustomNamesMissingArg)
{
    struct TestConfig : public Config {
        CMDLIME_ARG(argument, double) << cmdlime::Name{"customArg"};
        CMDLIME_ARGLIST(argumentList, std::vector<float>) << cmdlime::Name{"customArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { auto cfg = cfgReader.read<TestConfig>({}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'customArg' is missing."});
            });
}

TEST(SimpleConfig, CustomNamesMissingArgList)
{
    struct TestConfig : public Config {
        CMDLIME_ARG(argument, double) << cmdlime::Name{"customArg"};
        CMDLIME_ARGLIST(argumentList, std::vector<float>) << cmdlime::Name{"customArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ParsingError>(
            [&] { auto cfg = cfgReader.read<TestConfig>({"1.0"}); },
            [](const cmdlime::ParsingError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'customArgList' is missing."});
            });
}

TEST(SimpleConfig, ConfigErrorRepeatingParamNames)
{
    struct TestConfig : public Config {
        CMDLIME_PARAM(Prm, double)();
        CMDLIME_PARAM(prm, int)();
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
            [&] { cfgReader.read<TestConfig>({}); },
            [](const cmdlime::ConfigError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'prm' is already used."});
            });
}

TEST(SimpleConfig, ConfigErrorRepeatingFlagNames)
{
    struct TestConfig : public Config {
        CMDLIME_FLAG(Flg);
        CMDLIME_FLAG(flg);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    assert_exception<cmdlime::ConfigError>(
            [&] { cfgReader.read<TestConfig>({}); },
            [](const cmdlime::ConfigError& error) {
                EXPECT_EQ(std::string{error.what()}, std::string{"Flag's name 'flg' is already used."});
            });
}

TEST(SimpleConfig, UsageInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    cfgReader.setProgramName("testproc");
    auto expectedInfo = std::string{
            "Usage: testproc [commands] <argument> -requiredParam=<string> -prmList=<string>... "
            "[-optionalParam=<string>] [-optionalIntParam=<int>] [-optionalParamList=<int>...] [--flg] <argumentList...>\n"
    };
    EXPECT_EQ(cfgReader.usageInfo<FullConfig>(), expectedInfo);
}

TEST(SimpleConfig, DetailedUsageInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    cfgReader.setProgramName("testproc");
    auto expectedDetailedInfo = std::string{
            "Usage: testproc [commands] <argument> -requiredParam=<string> -prmList=<string>... [params] [flags] <argumentList...>\n"
            "Arguments:\n"
            "    <argument> (double)        \n"
            "    <argumentList> (float)     multi-value\n"
            "Parameters:\n"
            "   -requiredParam=<string>     \n"
            "   -prmList=<string>           multi-value\n"
            "   -optionalParam=<string>     optional, default: defaultValue\n"
            "   -optionalIntParam=<int>     optional\n"
            "   -optionalParamList=<int>    multi-value, optional, default: {99, 100}\n"
            "Flags:\n"
            "  --flg                        \n"
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
            "Usage: testproc [commands] <argument> -requiredParam=<string> -prmList=<string>... [params] [flags] <argumentList...>\n"
            "Arguments:\n"
            "<argument> (double)       \n"
            "<argumentList> (float)    multi-value\n"
            "Parameters:\n"
            "-requiredParam=<string>   \n"
            "-prmList=<string>         multi-value\n"
            "-optionalParam=<string>   optional, default: \n"
            "                            defaultValue\n"
            "-optionalIntParam=<int>   optional\n"
            "-optionalParamList=<int>  multi-value, optional,\n"
            "                            default: {99, 100}\n"
            "Flags:\n"
            "--flg                     \n"
            "Commands:\n"
            "subcommand [options]      \n"
    };
    EXPECT_EQ(cfgReader.usageInfoDetailed<FullConfig>(), expectedDetailedInfo);
}


TEST(SimpleConfig, CustomValueNames)
{
    struct TestConfig : public Config {
        CMDLIME_PARAM(prm, std::string) << cmdlime::ValueName{"STRING"};
        CMDLIME_PARAMLIST(prmList, std::vector<int>)() << cmdlime::ValueName{"INTS"};
        CMDLIME_ARG(argument, double) << cmdlime::ValueName{"DOUBLE"};
        CMDLIME_ARGLIST(argumentList, std::vector<float>) << cmdlime::ValueName{"FLOATS"};
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    cfgReader.setProgramName("testproc");
    auto expectedInfo = std::string{
            "Usage: testproc <argument> -prm=<STRING> [params] <argumentList...>\n"
            "Arguments:\n"
            "    <argument> (DOUBLE)         \n"
            "    <argumentList> (FLOATS)     multi-value\n"
            "Parameters:\n"
            "   -prm=<STRING>                \n"
            "   -prmList=<INTS>              multi-value, optional, default: {}\n"
    };
    EXPECT_EQ(cfgReader.usageInfoDetailed<TestConfig>(), expectedInfo);
}


TEST(SimpleConfig, WrongParamsWithExitFlag)
{
    struct ConfigWithExitFlag : public Config {
        CMDLIME_PARAM(requiredParam, int);
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
        CMDLIME_FLAG(flg);
        CMDLIME_EXITFLAG(exitFlg);
        CMDLIME_ARG(argument, double);
        CMDLIME_ARGLIST(argumentList, std::vector<float>);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{};
    auto cfg = cfgReader.read<ConfigWithExitFlag>({"asd", "asf", "--exitFlg"});
    EXPECT_EQ(cfg.exitFlg, true);
}

TEST(SimpleConfig, ExecMissingVersionInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    auto errorOutput = std::stringstream{};
    cfgReader.setErrorOutputStream(errorOutput);

    EXPECT_EQ(cfgReader.exec<FullConfig>({"--version"}, [](const FullConfig&) { return 0; }), -1);
    EXPECT_EQ(errorOutput.str(), "Encountered unknown flag '--version'\n");
}

TEST(SimpleConfig, ExecVersion)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    cfgReader.setVersionInfo("testproc 1.0");
    auto output = std::stringstream{};
    cfgReader.setOutputStream(output);
    EXPECT_EQ(cfgReader.exec<FullConfig>({"--version"}, [](const FullConfig&) { return 0; }), 0);
    EXPECT_EQ(output.str(), "testproc 1.0\n");
}

TEST(SimpleConfig, ExecHelp)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    auto output = std::stringstream{};
    cfgReader.setOutputStream(output);
    EXPECT_EQ(cfgReader.exec<FullConfig>({"--help"}, [](const FullConfig&) { return 0; }), 0);
    auto expectedDetailedInfo = std::string{
            "Usage: testproc [commands] <argument> -requiredParam=<string> -prmList=<string>... [params] [flags] <argumentList...>\n"
            "Arguments:\n"
            "    <argument> (double)        \n"
            "    <argumentList> (float)     multi-value\n"
            "Parameters:\n"
            "   -requiredParam=<string>     \n"
            "   -prmList=<string>           multi-value\n"
            "   -optionalParam=<string>     optional, default: defaultValue\n"
            "   -optionalIntParam=<int>     optional\n"
            "   -optionalParamList=<int>    multi-value, optional, default: {99, 100}\n"
            "Flags:\n"
            "  --flg                        \n"
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
    EXPECT_EQ(cfgReader.exec<FullConfig>({"--help"}, [](const FullConfig&) { return 0; }), 0);
    auto expectedDetailedInfo = std::string{
            "Usage: testproc [commands] <argument> -requiredParam=<string> -prmList=<string>... [params] [flags] <argumentList...>\n"
            "Arguments:\n"
            "<argument> (double)       \n"
            "<argumentList> (float)    multi-value\n"
            "Parameters:\n"
            "-requiredParam=<string>   \n"
            "-prmList=<string>         multi-value\n"
            "-optionalParam=<string>   optional, default: \n"
            "                            defaultValue\n"
            "-optionalIntParam=<int>   optional\n"
            "-optionalParamList=<int>  multi-value, optional,\n"
            "                            default: {99, 100}\n"
            "Flags:\n"
            "--flg                     \n"
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
            "Usage: testproc subcommand [commands] <argument> -requiredParam=<string> -prmList=<string>... [params] [flags] <argumentList...>\n"
            "Arguments:\n"
            "    <argument> (double)        \n"
            "    <argumentList> (float)     multi-value\n"
            "Parameters:\n"
            "   -requiredParam=<string>     \n"
            "   -prmList=<string>           multi-value\n"
            "   -optionalParam=<string>     optional, default: defaultValue\n"
            "   -optionalIntParam=<int>     optional\n"
            "   -optionalParamList=<int>    multi-value, optional, default: {99, 100}\n"
            "Flags:\n"
            "  --flg                        \n"
            "  --help                       show usage info and exit\n"
            "Commands:\n"
            "    nested [options]           \n\n"
    };
    EXPECT_EQ(cfgReader.exec<FullConfigWithCommand>({"subcommand", "--help"}, [](const auto&) { return 0; }), 0);
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
    EXPECT_EQ(cfgReader.exec<FullConfigWithCommand>({"subcommand", "--help"}, [](const auto&) { return 0; }), 0);
    auto expectedDetailedInfo = std::string{
            "Usage: testproc subcommand [commands] <argument> -requiredParam=<string> -prmList=<string>... [params] [flags] <argumentList...>\n"
            "Arguments:\n"
            "<argument> (double)       \n"
            "<argumentList> (float)    multi-value\n"
            "Parameters:\n"
            "-requiredParam=<string>   \n"
            "-prmList=<string>         multi-value\n"
            "-optionalParam=<string>   optional, default: \n"
            "                            defaultValue\n"
            "-optionalIntParam=<int>   optional\n"
            "-optionalParamList=<int>  multi-value, optional,\n"
            "                            default: {99, 100}\n"
            "Flags:\n"
            "--flg                     \n"
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
            "Usage: testproc subcommand nested -prm=<string> [flags] \n"
            "Parameters:\n"
            "   -prm=<string>     \n"
            "Flags:\n"
            "  --help             show usage info and exit\n\n"
    };
    EXPECT_EQ(cfgReader.exec<FullConfigWithCommand>({"subcommand", "nested", "--help"}, [](const auto&) { return 0; }),
              0);
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
            "Usage: testproc subcommand nested -prm=<string> [flags] \n"
            "Parameters:\n"
            "-prm=<string>   \n"
            "Flags:\n"
            "--help          show usage info and exit\n\n"
    };
    EXPECT_EQ(cfgReader.exec<FullConfigWithCommand>({"subcommand", "nested", "--help"}, [](const auto&) { return 0; }),
              0);
    EXPECT_EQ(output.str(), expectedDetailedInfo);
}

TEST(SimpleConfig, ExecSuccess)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    EXPECT_EQ(cfgReader.exec<FullConfig>(
            {"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-prmList=zero", "-prmList=one",
             "-optionalParamList=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"}, [](const auto&) { return 0; }), 0);
}

TEST(SimpleConfig, ExecError)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::Simple>{"testproc"};
    auto errorOutput = std::stringstream{};
    cfgReader.setErrorOutputStream(errorOutput);
    EXPECT_EQ(cfgReader.exec<FullConfig>({"-optionalParam=BAR", "-optionalIntParam=9", "-prmList=zero", "-prmList=one",
                                          "-optionalParamList=1,2", "--flg", "4.2", "1.1", "2.2", "3.3"},
                                         [](const auto&) { return 0; }), -1);
    EXPECT_EQ(errorOutput.str(), "Parameter '-requiredParam' is missing.\n");
}

}