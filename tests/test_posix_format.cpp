#include <gtest/gtest.h>
#include <cmdlime/config.h>
#include <cmdlime/commandlinereader.h>
#include "assert_exception.h"
#include <optional>

namespace test_posix_format{

using namespace cmdlime;

struct NestedSubcommandConfig: public Config{
    CMDLIME_PARAM(prm, std::string);
};

struct SubcommandConfig: public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::Name("i");
    CMDLIME_PARAMLIST(prmList, std::vector<std::string>) << cmdlime::Name("L");
    CMDLIME_PARAMLIST(optionalParamList, std::vector<int>)(std::vector<int>{99, 100}) << cmdlime::Name("O");
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, std::vector<float>);
    CMDLIME_COMMAND(nested, NestedSubcommandConfig);
};

struct FullConfig : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::Name("i");
    CMDLIME_PARAMLIST(prmList, std::vector<std::string>) << cmdlime::Name("L");
    CMDLIME_PARAMLIST(optionalParamList, std::vector<int>)(std::vector<int>{99, 100}) << cmdlime::Name("O");
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, std::vector<float>);
    CMDLIME_SUBCOMMAND(subcommand, SubcommandConfig);
};

struct FullConfigWithCommand : public Config{
    CMDLIME_PARAM(requiredParam, std::string);
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::Name("i");
    CMDLIME_PARAMLIST(prmList, std::vector<std::string>) << cmdlime::Name("L");
    CMDLIME_PARAMLIST(optionalParamList, std::vector<int>)(std::vector<int>{99, 100}) << cmdlime::Name("O");
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, std::vector<float>);
    CMDLIME_COMMAND(subcommand, SubcommandConfig);
};


TEST(PosixConfig, AllSet)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::POSIX>{};
    auto cfg = reader.read<FullConfig>({"-r", "FOO", "-oBAR", "-i", "-9", "-L", "zero", "-L", "one",
                                           "-O", "1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
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

TEST(PosixConfig, AllSetInSubCommand)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<FullConfig>({"-r", "FOO", "-L", "zero", "-L", "one",
                                           "4.2", "1.1",
                                           "subcommand", "-r", "FOO", "-oBAR", "-i", "9", "-L", "zero", "-L", "one",
                                           "-O", "1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam);
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

TEST(PosixConfig, CombinedFlagsAndParams)
{
    struct Cfg : public Config{
        CMDLIME_FLAG(firstFlag);
        CMDLIME_FLAG(secondFlag);
        CMDLIME_FLAG(thirdFlag);
        CMDLIME_PARAM(prm, std::string)();
    };

    {
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<Cfg>({"-fst", "-pfirst"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.prm, "first");
    }

    {
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<Cfg>({"-tfspfirst"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.prm, "first");
    }

    {
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<Cfg>({"-fs", "-tp" "first"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, true);
    EXPECT_EQ(cfg.prm, "first");
    }

    {
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<Cfg>({"-fs", "-p", "first"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, false);
    EXPECT_EQ(cfg.prm, "first");
    }
}

TEST(PosixConfig, NumericParamsAndFlags)
{
    struct Cfg : public Config{
        CMDLIME_FLAG(flg) << cmdlime::Name("1");
        CMDLIME_PARAM(prm, std::string) << cmdlime::Name("2");
        CMDLIME_PARAM(paramSecond, std::string)("default");
        CMDLIME_ARG(argument, int);
    };
    {
        auto reader = cmdlime::POSIXCommandLineReader{};
        auto cfg = reader.read<Cfg>({"-1", "-2", "test", "-99"});
        EXPECT_EQ(cfg.flg, true);
        EXPECT_EQ(cfg.prm, "test");
        EXPECT_EQ(cfg.paramSecond, "default");
        EXPECT_EQ(cfg.argument, -99);
    }
    {
        auto reader = cmdlime::POSIXCommandLineReader{};
        auto cfg = reader.read<Cfg>({"-2test", "-1", "-p", "second", "-99"});
        EXPECT_EQ(cfg.flg, true);
        EXPECT_EQ(cfg.prm, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.argument, -99);
    }
    {
        auto reader = cmdlime::POSIXCommandLineReader{};
        auto cfg = reader.read<Cfg>({"-12test", "-p", "second", "-99"});
        EXPECT_EQ(cfg.flg, true);
        EXPECT_EQ(cfg.prm, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.argument, -99);
    }
}

TEST(PosixConfig, MissingOptionals)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<FullConfig>({"-r", "FOO", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}


TEST(PosixConfig, MissingParamAllSetInSubCommand)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>(
                {"subcommand", "-r", "FOO", "-oBAR", "-i", "9", "-L", "zero", "-L", "one",
                 "-O", "1,2", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-r' is missing."});
        });
}

TEST(PosixConfig, AllSetInCommand)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<FullConfigWithCommand>({"-r", "FOO", "-L", "zero", "-L", "one", "4.2", "1.1",
                                                      "subcommand", "-r", "FOO", "-oBAR", "-i", "9", "-L", "zero", "-L",
                                                      "one",
                                                      "-O", "1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam);
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

TEST(PosixConfig, MissingParamAllSetInCommand)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<FullConfigWithCommand>(
            {"subcommand", "-r", "FOO", "-oBAR", "-i", "9", "-L", "zero", "-L", "one",
             "-O", "1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam);
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

TEST(PosixConfig, MissingParamAllSetInNestedCommand)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<FullConfigWithCommand>({"subcommand", "nested", "-p", "FOO"});
    EXPECT_TRUE(cfg.requiredParam.empty());
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.optionalIntParam);
    EXPECT_TRUE(cfg.prmList.empty());
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{99,100}));
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 0.f);
    EXPECT_TRUE(cfg.argumentList.empty());
    ASSERT_TRUE(cfg.subcommand);
    EXPECT_TRUE(cfg.subcommand->requiredParam.empty());
    EXPECT_EQ(cfg.subcommand->optionalParam, std::string{"defaultValue"});
    EXPECT_FALSE(cfg.subcommand->optionalIntParam);
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
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::Name("i");
    CMDLIME_FLAG(flg);
    CMDLIME_ARG(argument, double);
    CMDLIME_ARGLIST(argumentList, std::vector<float>)({1.f, 2.f}) << cmdlime::Name("A");
};

TEST(PosixConfig, MissingOptionalArgList)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<FullConfigWithOptionalArgList>({"-r", "FOO", "4.2"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.flg, false);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.f, 2.f}));
}

TEST(PosixConfig, MissingParam)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>({"-o", "FOO", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-r' is missing."});
        });
}

TEST(PosixConfig, MissingArg)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfigWithOptionalArgList>({"-r", "FOO", "-o", "BAR", "-i", "9", "-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'argument' is missing."});
        });
}

TEST(PosixConfig, MissingArgList)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>({"-r", "FOO", "-o", "BAR", "-i", "9", "-L", "zero", "-f", "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'argument-list' is missing."});
        });
}

TEST(PosixConfig, MissingParamList)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "-i", "9", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-L' is missing."});
        });
}

TEST(PosixConfig, UnexpectedParam)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>({"-r", "FOO", "-t", "TEST", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-t'"});
        });
}

TEST(PosixConfig, UnexpectedFlag)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>({"-r", "FOO", "-t", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-t'"});
        });
}

TEST(PosixConfig, UnexpectedArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string);
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<Cfg>({"-p", "FOO", "4.2", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
        });
}

TEST(PosixConfig, WrongCommandsOrder)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>);
    };

    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ auto cfg = reader.read<Cfg>({"0", "1", "-o", "1", "2", "--"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Flags and parameters must precede arguments"});
        });
}

TEST(PosixConfig, WrongParamType)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "-i", "nine", "-L", "zero", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-i' value from 'nine'"});
        });
}

TEST(PosixConfig, WrongParamListElementType)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "-L", "zero", "-O", "not-int", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-O' value from 'not-int'"});
        });
}

TEST(PosixConfig, WrongArgType)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "-i", "9", "-L", "zero", "-f", "fortytwo", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'argument' value from 'fortytwo'"});
        });
}

TEST(PosixConfig, WrongArgListElementType)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<FullConfig>(
                {"-r", "FOO", "-o", "BAR", "-i", "9", "-L", "zero", "-f", "4.2", "1.1", "2.2", "three"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument list 'argument-list' element's value from 'three'"});
        });
}

TEST(PosixConfig, ParamEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string);
        CMDLIME_FLAG(flg);
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<Cfg>({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
}

TEST(PosixConfig, ParamListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_PARAMLIST(params, std::vector<std::string>);
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<Cfg>({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
}

TEST(PosixConfig, ArgEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARG(argument, std::string);
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<Cfg>({""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg 'argument' value can't be empty"});
        });
}

TEST(PosixConfig, ArgListEmptyValue)
{
    struct Cfg : public Config{
        CMDLIME_ARGLIST(args, std::vector<std::string>);
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<Cfg>({"foo", ""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg list 'args' element value can't be empty"});
        });
}


TEST(PosixConfig, ValuesWithWhitespace)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string);
        CMDLIME_PARAMLIST(prmList, std::vector<std::string>) << cmdlime::Name("L");
        CMDLIME_ARG(argument, std::string);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>) << cmdlime::Name("A");
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<Cfg>({"-p", "Hello world", "-L", "foo bar", "foo bar", "forty two"});
    EXPECT_EQ(cfg.prm, "Hello world");
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"foo bar"}));
    EXPECT_EQ(cfg.argument, "foo bar");
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"forty two"}));
}

TEST(PosixConfig, ParamWrongNameTooLong)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::Name("prm");
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ConfigError>(
        [&]{auto cfg = reader.read<Cfg>({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'prm' can't have more than one symbol"});
        });
}

TEST(PosixConfig, ParamWrongNameNonAlphanum)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, std::string) << cmdlime::Name("$");
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ConfigError>(
        [&]{auto cfg = reader.read<Cfg>({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name '$' must be an alphanumeric character"});
        });
}

TEST(PosixConfig, NegativeNumberToArg)
{
    struct Cfg : public Config{
        CMDLIME_ARG(argument, int);
        CMDLIME_ARG(argumentStr, std::string);
        CMDLIME_ARGLIST(argumentList, std::vector<double>);
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<Cfg>({"-2", "-3", "4.5", "-6.7"});
    EXPECT_EQ(cfg.argument, -2);
    EXPECT_EQ(cfg.argumentStr, "-3");
    EXPECT_EQ(cfg.argumentList, (std::vector<double>{4.5, -6.7}));
}

TEST(PosixConfig, NegativeNumberWithoutArg)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, int);
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ reader.read<Cfg>({"-2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '-2'"});
        });
}

TEST(PosixConfig, ArgsDelimiter)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(prm, int);
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>);
    };

    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<Cfg>({"-p", "11", "0", "--", "1", "-o", "2"});
    EXPECT_EQ(cfg.prm, 11);
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "-o", "2"}));
}

TEST(PosixConfig, ArgsDelimiterFront)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>);
    };

    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<Cfg>({"--", "0", "1", "-o", "1", "2"});
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "-o", "1", "2"}));
}

TEST(PosixConfig, ArgsDelimiterBack)
{
    struct Cfg : public Config{
        CMDLIME_PARAM(optionalParam, int)(0);
        CMDLIME_ARGLIST(argumentList, std::vector<std::string>);
    };

    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<Cfg>({"-o", "1", "0", "1", "2", "--"});
    EXPECT_EQ(cfg.optionalParam, 1);
    EXPECT_EQ(cfg.argumentList, (std::vector<std::string>{"0", "1", "2"}));
}

TEST(PosixConfig, PascalNames)
{
    struct PascalConfig : public Config{
        CMDLIME_PARAM(RequiredParam, std::string);
        CMDLIME_PARAM(OptionalParam, std::string)("defaultValue");
        CMDLIME_PARAM(IntParamOptional, std::optional<int>)();
        CMDLIME_PARAMLIST(ListOfParam, std::vector<std::string>);
        CMDLIME_PARAMLIST(MyListOfParamOptional, std::vector<int>)(std::vector<int>{99, 100});
        CMDLIME_FLAG(Flag);
        CMDLIME_ARG(argument, double);
        CMDLIME_ARGLIST(argumentList, std::vector<float>);
    };
   auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<PascalConfig>({"-r", "FOO", "-o", "BAR", "-i", "9", "-l", "zero", "-l", "one",
                                             "-m", "1", "-m", "2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.RequiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.OptionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.IntParamOptional, 9);
    EXPECT_EQ(cfg.ListOfParam, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.MyListOfParamOptional, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.Flag, true);
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(PosixConfig, CustomNamesMissingParam)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(prm, double) << cmdlime::Name{"P"};
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{ reader.read<TestConfig>({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-P' is missing."});
        });
}

TEST(PosixConfig, CustomNamesMissingParamList)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(prm, double) << cmdlime::Name{"P"};
        CMDLIME_PARAMLIST(prmList, std::vector<float>) << cmdlime::Name{"L"};
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<TestConfig>({"-P1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-L' is missing."});
        });
}

TEST(PosixConfig, CustomNamesMissingArg)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(argument, double) << cmdlime::Name{"a"};
        CMDLIME_ARGLIST(argumentList, std::vector<float>) << cmdlime::Name{"A"};
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<TestConfig>({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'a' is missing."});
        });
}

TEST(PosixConfig, CustomNamesMissingArgList)
{
    struct TestConfig : public Config{
        CMDLIME_ARG(argument, double) << cmdlime::Name{"a"};
        CMDLIME_ARGLIST(argumentList, std::vector<float>) << cmdlime::Name{"A"};
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ParsingError>(
        [&]{auto cfg = reader.read<TestConfig>({"1.0"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'A' is missing."});
        });
}

TEST(PosixConfig, ConfigErrorRepeatingParamNames)
{
    struct TestConfig : public Config{
        CMDLIME_PARAM(Prm, double)();
        CMDLIME_PARAM(prm, int)();
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    assert_exception<cmdlime::ConfigError>(
        [&]{ reader.read<TestConfig>({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'p' is already used."});
        });
}

TEST(PosixConfig, UsageInfo)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    reader.setProgramName("testproc");
    auto expectedInfo = std::string{
    "Usage: testproc [commands] <argument> -r <string> -L <string>... "
    "[-o <string>] [-i <int>] [-O <int>...] [-f] <argument-list...>\n"
    };
    EXPECT_EQ(reader.usageInfo<FullConfig>(), expectedInfo);
}

TEST(PosixConfig, DetailedUsageInfo)
{
    auto reader = cmdlime::POSIXCommandLineReader{};
    reader.setProgramName("testproc");
    auto expectedDetailedInfo = std::string{
    "Usage: testproc [commands] <argument> -r <string> -L <string>... [params] [flags] <argument-list...>\n"
    "Arguments:\n"
    "    <argument> (double)         \n"
    "    <argument-list> (float)     multi-value\n"
    "Parameters:\n"
    "   -r <string>                  \n"
    "   -L <string>                  multi-value\n"
    "   -o <string>                  optional, default: defaultValue\n"
    "   -i <int>                     optional\n"
    "   -O <int>                     multi-value, optional, default: {99, 100}\n"
    "Flags:\n"
    "   -f                           \n"
    "Commands:\n"
    "    subcommand [options]        \n"};
    EXPECT_EQ(reader.usageInfoDetailed<FullConfig>(), expectedDetailedInfo);
}

TEST(PosixConfig, WrongParamsWithExitFlag){
    struct ConfigWithExitFlag : public Config{
        CMDLIME_PARAM(requiredParam, int);
        CMDLIME_PARAM(optionalParam, std::string)("defaultValue");
        CMDLIME_FLAG(flg);
        CMDLIME_EXITFLAG(exitFlg);
        CMDLIME_ARG(argument, double);
        CMDLIME_ARGLIST(argumentList, std::vector<float>);
    };
    auto reader = cmdlime::POSIXCommandLineReader{};
    auto cfg = reader.read<ConfigWithExitFlag>({"-asd", "-asf", "-e"});
    EXPECT_EQ(cfg.exitFlg, true);
}

}
