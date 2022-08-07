#include <gtest/gtest.h>
#include <cmdlime/config.h>
#include <cmdlime/configreader.h>
#include "assert_exception.h"
#include <optional>

namespace text_x11_format{

using namespace cmdlime;

struct NestedSubcommandConfig: public Config{
    CMDLIME_PARAM(prm, std::string);
};

struct SubcommandConfig: public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_PARAMLIST(prmList, std::string);
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100});
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, float);
    CMDLIME_COMMAND(nested, NestedSubcommandConfig);
};

struct FullConfig : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_PARAMLIST(prmList, std::string);
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100});
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, float);
    CMDLIME_SUBCOMMAND(subcommand, SubcommandConfig);
};

struct FullConfigWithCommand : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_PARAMLIST(prmList, std::string);
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100});
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, float);
    CMDLIME_COMMAND(subcommand, SubcommandConfig);
};

TEST(X11Config, AllSet)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<FullConfig>(
            {"-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "-9", "-prmlist", "zero",
             "-prmlist", "one",
             "-optionalparamlist", "1,2", "-flg", "4.2", "1.1", "2.2", "3.3"});
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

TEST(X11Config, AllSetInSubCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<FullConfig>(
            {"-requiredparam", "FOO", "-prmlist", "zero", "-prmlist", "one", "4.2", "1.1",
             "subcommand", "-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-prmlist",
             "zero", "-prmlist", "one",
             "-optionalparamlist", "1,2", "-flg", "4.2", "1.1", "2.2", "3.3"});
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

TEST(X11Config, MissingOptionals)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<FullConfig>({"-requiredparam", "FOO", "-prmlist", "zero", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(X11Config, MissingParamAllSetInSubCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"subcommand", "-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-prmlist",
                 "zero", "-prmlist", "one",
                 "-optionalparamlist", "1,2", "-flg", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-requiredparam' is missing."});
        });
}

TEST(X11Config, AllSetInCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<FullConfigWithCommand>(
            {"-requiredparam", "FOO", "-prmlist", "zero", "-prmlist", "one", "4.2", "1.1",
             "subcommand", "-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-prmlist",
             "zero", "-prmlist", "one",
             "-optionalparamlist", "1,2", "-flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.prmList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
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

TEST(X11Config, MissingParamAllSetInCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<FullConfigWithCommand>(
            {"subcommand", "-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-prmlist",
             "zero", "-prmlist", "one",
             "-optionalparamlist", "1,2", "-flg", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.prmList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
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

TEST(X11Config, MissingParamAllSetInNestedCommand)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<FullConfigWithCommand>({"subcommand", "nested", "-prm", "FOO"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam.has_value());
    EXPECT_TRUE(cfg.prmList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 0.f);
    EXPECT_TRUE(cfg.argumentList.empty());
    ASSERT_TRUE(cfg.subcommand);
    EXPECT_TRUE(cfg.subcommand->requiredParam.empty());
    EXPECT_EQ(cfg.subcommand->optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.subcommand->optionalIntParam.has_value());
    EXPECT_TRUE(cfg.subcommand->prmList.empty());
    EXPECT_EQ(cfg.subcommand->optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.subcommand->flg, false);
    EXPECT_EQ(cfg.subcommand->argument, 0.f);
    EXPECT_TRUE(cfg.subcommand->argumentList.empty());
    ASSERT_TRUE(cfg.subcommand->nested);
    EXPECT_EQ(cfg.subcommand->nested->prm, "FOO");
}

struct FullConfigWithOptionalArgList : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)({"defaultValue"});
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)();
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, float)({1.f, 2.f});
};

TEST(X11Config, MissingOptionalArgList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<FullConfigWithOptionalArgList>({"-requiredparam", "FOO", "4.2"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.f, 2.f}));
}

TEST(X11Config, MissingParam)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-optionalparam", "FOO", "-prmlist", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-requiredparam' is missing."});
        });
}

TEST(X11Config, MissingArg)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfigWithOptionalArgList>(
                {"-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-flg"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'argument' is missing."});
        });
}

TEST(X11Config, MissingArgList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-prmlist", "zero",
                 "-flg", "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'argumentlist' is missing."});
        });
}

TEST(X11Config, MissingParamList)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-flg", "4.2", "1.1",
                 "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-prmlist' is missing."});
        });
}

TEST(X11Config, UnexpectedParam)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-requiredparam", "FOO", "-testparam", "TEST", "-prmlist", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-testparam'"});
        });
}

TEST(X11Config, UnexpectedFlag)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-requiredparam", "FOO", "-testflag", "-prmlist", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-testflag'"});
        });
}

TEST(X11Config, UnexpectedArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-prm", "FOO", "4.2", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
        });

}

TEST(X11Config, WrongParamType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "nine", "-prmlist", "zero",
                 "-flg", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-optionalintparam' value from 'nine'"});
        });
}

TEST(X11Config, WrongParamListElementType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-requiredparam", "FOO", "-optionalparam", "BAR", "-prmlist", "zero", "-optionalparamlist",
                 "not-int", "-flg", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-optionalparamlist' value from 'not-int'"});
        });
}

TEST(X11Config, WrongArgType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-prmlist", "zero",
                 "-flg", "fortytwo", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'argument' value from 'fortytwo'"});
        });
}

TEST(X11Config, WrongArgListElementType)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<FullConfig>(
                {"-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-prmlist", "zero",
                 "-flg", "4.2", "1.1", "2.2", "three"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument list 'argumentlist' element's value from 'three'"});
        });
}

TEST(X11Config, ParamWrongNameNonAlphaFirstChar)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::Name("!param");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-param", "name"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name '!param' must start with an alphabet character"});
        });
}

TEST(X11Config, ParamWrongNameNonAlphanum)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::Name("p$r$m");
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-param", "name"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'p$r$m' must consist of alphanumeric characters and hyphens"});
        });
}

TEST(X11Config, ParamEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-prm"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-prm' value can't be empty"});
        });
}

TEST(X11Config, ParamListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAMLIST(params, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"-params", ""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-params' value can't be empty"});
        });
}

TEST(X11Config, ArgEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARG(argument, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg 'argument' value can't be empty"});
        });
}

TEST(X11Config, ArgListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARGLIST(args, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = cfgReader.read<Cfg>({"foo", ""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg list 'args' element value can't be empty"});
        });
}


TEST(X11Config, ValuesWithWhitespace)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string);
        CMDLIME_PARAMLIST(prmList, std::string);
        CMDLIME_ARG(argument, std::string);
        CMDLIME_ARGLIST(argumentList, std::string);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<Cfg>({"-prm", "Hello world", "-prmlist", "foo bar", "foo bar", "forty two"});
    EXPECT_EQ(cfg.prm, "Hello world");
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"foo bar"}));
    EXPECT_EQ(cfg.argument, "foo bar");
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"forty two"}));
}

TEST(X11Config, NegativeNumberToArg)
{
    struct Cfg : public Config{
        CMDLIME_ARG(argument, int);
        CMDLIME_ARG(argumentStr, std::string);
        CMDLIME_ARGLIST(argumentList, double);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<Cfg>({"-2", "-3", "4.5", "-6.7"});
    EXPECT_EQ(cfg.argument, -2);
    EXPECT_EQ(cfg.argumentStr, "-3");
    EXPECT_EQ(cfg.argumentList, (std::vector<double>{4.5, -6.7}));
}

TEST(X11Config, NegativeNumberWithoutArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, int);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<Cfg>({"-2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '-2'"});
        });
}

TEST(X11Config, ArgsDelimiter)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, int);
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::string);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<Cfg>({"0", "-prm", "11", "--", "1", "-optionalparam", "2"});
    EXPECT_EQ(cfg.prm, 11);
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "-optionalparam", "2"}));
}

TEST(X11Config, ArgsDelimiterFront)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::string);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<Cfg>({"--", "0", "1", "-optionalparam", "1", "2"});
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "-optionalparam","1", "2"}));
}

TEST(X11Config, ArgsDelimiterBack)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::string);
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<Cfg>({"0", "1", "-optionalparam", "1", "2", "--"});
    EXPECT_EQ(cfg.optionalParam, 1);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "2"}));
}

TEST(X11Config, PascalNames)
{
    struct PascalConfig : public Config{
        CMDLIME_PARAM(RequiredParam, std::string);
        CMDLIME_PARAM(OptionalParam, std::string)("defaultValue");
        CMDLIME_PARAM(OptionalIntParam, std::optional<int>)();
        CMDLIME_PARAMLIST(prmList, std::string);
        CMDLIME_PARAMLIST(OptionalParamList, int)(std::vector<int>{99, 100});
        CMDLIME_FLAG(Flag);
        CMDLIME_ARG(argument, double);
        CMDLIME_ARGLIST(argumentList, float);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<PascalConfig>(
            {"-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-prmlist", "zero",
             "-prmlist", "one",
             "-optionalparamlist", "1", "-optionalparamlist", "2", "-flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.RequiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.OptionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.OptionalIntParam, 9);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.OptionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.Flag, true);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(X11Config, SnakeNames)
{
    struct PascalConfig : public Config{
        CMDLIME_PARAM(required_param, std::string);
        CMDLIME_PARAM(optional_param, std::string)("defaultValue");
        CMDLIME_PARAM(optional_int_param, std::optional<int>)();
        CMDLIME_PARAMLIST(param_list, std::string);
        CMDLIME_PARAMLIST(optional_param_list_, int)(std::vector<int>{99, 100});
        CMDLIME_FLAG(flag_);
        CMDLIME_ARG(argument_, double);
        CMDLIME_ARGLIST(arg_list_, float);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<PascalConfig>(
            {"-requiredparam", "FOO", "-optionalparam", "BAR", "-optionalintparam", "9", "-paramlist", "zero",
             "-paramlist", "one",
             "-optionalparamlist", "1", "-optionalparamlist", "2", "-flag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.required_param, std::string{"FOO"});
    EXPECT_EQ(cfg.optional_param, std::string{"BAR"});
    EXPECT_EQ(cfg.optional_int_param, 9);
    EXPECT_EQ(cfg.param_list, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optional_param_list_, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flag_, true);
    EXPECT_EQ(cfg.argument_, 4.2);
    EXPECT_EQ(cfg.arg_list_, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(X11Config, CustomNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(requiredParam, std::string) << cmdlime::Name{"customRequiredParam"};
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue") << cmdlime::Name{"customOptionalParam"};
        CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::Name{"customOptionalIntParam"};
        CMDLIME_FLAG(flg) << cmdlime::Name{"customFlag"};
        CMDLIME_ARG(argument, double) << cmdlime::Name{"customArg"};
        CMDLIME_ARGLIST(argumentList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<TestConfig>(
            {"-customRequiredParam", "FOO", "-customOptionalParam", "BAR", "-customOptionalIntParam", "9",
             "-customFlag", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.flg, true);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(X11Config, CustomNamesMissingParam)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(prm, double) << cmdlime::Name{"customParam"};
        CMDLIME_PARAMLIST(prmList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<TestConfig>({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-customParam' is missing."});
        });
}

TEST(X11Config, CustomNamesMissingParamList)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(prm, double) << cmdlime::Name{"customParam"};
        CMDLIME_PARAMLIST(prmList, float) << cmdlime::Name{"customParamList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<TestConfig>({"-customParam", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-customParamList' is missing."});
        });
}

TEST(X11Config, CustomNamesMissingArg)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(argument, double) << cmdlime::Name{"customArg"};
        CMDLIME_ARGLIST(argumentList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<TestConfig>({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'customArg' is missing."});
        });
}

TEST(X11Config, CustomNamesMissingArgList)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(argument, double) << cmdlime::Name{"customArg"};
        CMDLIME_ARGLIST(argumentList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ cfgReader.read<TestConfig>({"1.0"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'customArgList' is missing."});
        });
}

TEST(X11Config, ConfigErrorRepeatingParamNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(Prm, double)();
        CMDLIME_PARAM(prm, int)();
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ cfgReader.read<TestConfig>({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'prm' is already used."});
        });
}

TEST(X11Config, ConfigErrorRepeatingFlagNames)
{
    struct TestConfig : public Config{
        CMDLIME_FLAG(Flg);
        CMDLIME_FLAG(flg);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ cfgReader.read<TestConfig>({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Flag's name 'flg' is already used."});
        });
}

TEST(X11Config, UsageInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    cfgReader.setProgramName("testproc");
    auto expectedInfo = std::string{
    "Usage: testproc [commands] <argument> -requiredparam <string> -prmlist <string>... "
    "[-optionalparam <string>] [-optionalintparam <int>] [-optionalparamlist <int>...] [-flg] <argumentlist...>\n"
    };
    EXPECT_EQ(cfgReader.usageInfo<FullConfig>(), expectedInfo);
}

TEST(X11Config, DetailedUsageInfo)
{
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    cfgReader.setProgramName("testproc");
    auto expectedDetailedInfo = std::string{
    "Usage: testproc [commands] <argument> -requiredparam <string> -prmlist <string>... [params] [flags] <argumentlist...>\n"
    "Arguments:\n"
    "    <argument> (double)        \n"
    "    <argumentlist> (float)     multi-value\n"
    "Parameters:\n"
    "   -requiredparam <string>     \n"
    "   -prmlist <string>           multi-value\n"
    "   -optionalparam <string>     optional, default: defaultValue\n"
    "   -optionalintparam <int>     optional\n"
    "   -optionalparamlist <int>    multi-value, optional, default: {99, 100}\n"
    "Flags:\n"
    "   -flg                        \n"
    "Commands:\n"
    "    subcommand [options]       \n"};
    EXPECT_EQ(cfgReader.usageInfoDetailed<FullConfig>(), expectedDetailedInfo);
}

TEST(X11Config, CustomValueNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::ValueName{"STRING"};
        CMDLIME_PARAMLIST(prmList, int)() << cmdlime::ValueName{"INTS"};
        CMDLIME_ARG(argument, double) << cmdlime::ValueName{"DOUBLE"};
        CMDLIME_ARGLIST(argumentList, float) << cmdlime::ValueName{"FLOATS"};
    };

    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    cfgReader.setProgramName("testproc");
    auto expectedInfo = std::string{
    "Usage: testproc <argument> -prm <STRING> [params] <argumentlist...>\n"
    "Arguments:\n"
    "    <argument> (DOUBLE)         \n"
    "    <argumentlist> (FLOATS)     multi-value\n"
    "Parameters:\n"
    "   -prm <STRING>                \n"
    "   -prmlist <INTS>              multi-value, optional, default: {}\n"
    };
    EXPECT_EQ(cfgReader.usageInfoDetailed<TestConfig>(), expectedInfo);
}

TEST(X11Config, WrongParamsWithExitFlag){
    struct ConfigWithExitFlag : public Config{
        CMDLIME_PARAM(requiredParam, int);
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
        CMDLIME_FLAG(flg);
        CMDLIME_EXITFLAG(exitFlg);
        CMDLIME_ARG(argument, double);
        CMDLIME_ARGLIST(argumentList, float);
    };
    auto cfgReader = cmdlime::ConfigReader<cmdlime::Format::X11>{};
    auto cfg = cfgReader.read<ConfigWithExitFlag>({"asd", "asf", "-exitflg"});
    EXPECT_EQ(cfg.exitFlg, true);
}

}
