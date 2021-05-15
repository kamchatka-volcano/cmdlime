#include <gtest/gtest.h>
#include <cmdlime/posixconfig.h>
#include "assert_exception.h"
#include <optional>

namespace test_posix_format{

using Config = cmdlime::POSIXConfig;

struct FullConfig : public Config{
    PARAM(requiredParam, std::string);
    PARAM(optionalParam, std::string)("defaultValue");
    PARAM(optionalIntParam, std::optional<int>)()               << cmdlime::Name("i");
    PARAMLIST(paramList, std::string)                           << cmdlime::Name("L");
    PARAMLIST(optionalParamList, int)(std::vector<int>{99,100}) << cmdlime::Name("O");
    FLAG(flag);
    ARG(arg, double);
    ARGLIST(argList, float);
};

TEST(PosixConfig, AllSet)
{
    auto cfg = FullConfig{};
    cfg.read({"-r", "FOO", "-oBAR", "-i", "9", "-L","zero", "-L", "one",
              "-O", "1,2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flag, true);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(PosixConfig, CombinedFlagsAndParams)
{
    struct Cfg : public Config{
        FLAG(firstFlag);
        FLAG(secondFlag);
        FLAG(thirdFlag);
        PARAM(param, std::string)();
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
    cfg.read({"-fs", "-p", "first"});
    EXPECT_EQ(cfg.firstFlag, true);
    EXPECT_EQ(cfg.secondFlag, true);
    EXPECT_EQ(cfg.thirdFlag, false);
    EXPECT_EQ(cfg.param, "first");
    }
}

TEST(PosixConfig, NumericParamsAndFlags)
{
    struct Cfg : public Config{
        FLAG(flag) << cmdlime::Name("1");
        PARAM(param, std::string) << cmdlime::Name("2");
        PARAM(paramSecond, std::string)("default");
        ARG(arg, int);
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
        cfg.read({"-2test", "-1", "-p", "second", "-99"});
        EXPECT_EQ(cfg.flag, true);
        EXPECT_EQ(cfg.param, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.arg, -99);
    }
    {
        auto cfg = Cfg{};
        cfg.read({"-12test", "-p", "second", "-99"});
        EXPECT_EQ(cfg.flag, true);
        EXPECT_EQ(cfg.param, "test");
        EXPECT_EQ(cfg.paramSecond, "second");
        EXPECT_EQ(cfg.arg, -99);
    }
}

TEST(PosixConfig, MissingOptionals)
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

struct FullConfigWithOptionalArgList : public Config{
    PARAM(requiredParam, std::string);
    PARAM(optionalParam, std::string)({"defaultValue"});
    PARAM(optionalIntParam, std::optional<int>)() << cmdlime::Name("i");
    FLAG(flag);
    ARG(arg, double);
    ARGLIST(argList, float)({1.f, 2.f}) << cmdlime::Name("A");
};

TEST(PosixConfig, MissingOptionalArgList)
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

TEST(PosixConfig, MissingParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-o", "FOO","-L","zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-r' is missing."});
        });
}

TEST(PosixConfig, MissingArg)
{
    auto cfg = FullConfigWithOptionalArgList{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o","BAR", "-i","9", "-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'arg' is missing."});
        });
}

TEST(PosixConfig, MissingArgList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o","BAR", "-i","9","-L","zero","-f", "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'arg-list' is missing."});
        });
}

TEST(PosixConfig, MissingParamList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r","FOO", "-o","BAR", "-i","9","-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-L' is missing."});
        });
}

TEST(PosixConfig, UnexpectedParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r","FOO", "-t","TEST","-L","zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-t'"});
        });
}

TEST(PosixConfig, UnexpectedFlag)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-t", "-L", "zero", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter or flag '-t'"});
        });
}

TEST(PosixConfig, UnexpectedArg)
{
    struct Cfg : public Config{
        PARAM(param, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-p","FOO", "4.2", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
        });
}

TEST(PosixConfig, WrongCommandsOrder)
{
    struct Cfg : public Config{
        PARAM(optionalParam, int)(0);
        ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{ cfg.read({"0", "1","-o","1", "2", "--"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Flags and parameters must preceed arguments"});
        });
}

TEST(PosixConfig, WrongParamType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o","BAR", "-i","nine", "-L","zero", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-i' value from 'nine'"});
        });
}

TEST(PosixConfig, WrongParamListElementType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r","FOO", "-o", "BAR", "-L", "zero", "-O", "not-int", "-f", "4.2", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter '-O' value from 'not-int'"});
        });
}

TEST(PosixConfig, WrongArgType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o", "BAR", "-i","9","-L","zero", "-f", "fortytwo", "1.1", "2.2", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'arg' value from 'fortytwo'"});
        });
}

TEST(PosixConfig, WrongArgListElementType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-r", "FOO", "-o","BAR", "-i","9", "-L", "zero", "-f", "4.2", "1.1", "2.2", "three"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument list 'arg-list' element's value from 'three'"});
        });
}

TEST(PosixConfig, ParamEmptyValue)
{
    struct Cfg : public Config{
        PARAM(param, std::string);
        FLAG(flag);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-p", "-f"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
}

TEST(PosixConfig, ParamListEmptyValue)
{
    struct Cfg : public Config{
        PARAMLIST(params, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-p"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-p' value can't be empty"});
        });
}

TEST(PosixConfig, ArgEmptyValue)
{
    struct Cfg : public Config{
        ARG(arg, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg 'arg' value can't be empty"});
        });
}

TEST(PosixConfig, ArgListEmptyValue)
{
    struct Cfg : public Config{
        ARGLIST(args, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"foo", ""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arg list 'args' element value can't be empty"});
        });
}


TEST(PosixConfig, ValuesWithWhitespace)
{
    struct Cfg : public Config{
        PARAM(param, std::string);
        PARAMLIST(paramList, std::string) << cmdlime::Name("L");
        ARG(arg, std::string);
        ARGLIST(argList, std::string) << cmdlime::Name("A");
    };
    auto cfg = Cfg{};
    cfg.read({"-p", "Hello world", "-L", "foo bar", "foo bar", "forty two"});
    EXPECT_EQ(cfg.param, "Hello world");
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"foo bar"}));
    EXPECT_EQ(cfg.arg, "foo bar");
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"forty two"}));
}

TEST(PosixConfig, ParamWrongNameTooLong)
{
    struct Cfg : public Config{
        PARAM(param, std::string) << cmdlime::Name("prm");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.read({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name can't have more than one symbol"});
        });
}

TEST(PosixConfig, ParamWrongNameNonAlphanum)
{
    struct Cfg : public Config{
        PARAM(param, std::string) << cmdlime::Name("$");
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.read({"-pname"});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name must be an alphanumeric character"});
        });
}

TEST(PosixConfig, NegativeNumberToArg)
{
    struct Cfg : public Config{
        ARG(arg, int);
        ARG(argStr, std::string);
        ARGLIST(argList, double);
    };
    auto cfg = Cfg{};
    cfg.read({"-2", "-3", "4.5", "-6.7"});
    EXPECT_EQ(cfg.arg, -2);
    EXPECT_EQ(cfg.argStr, "-3");
    EXPECT_EQ(cfg.argList, (std::vector<double>{4.5, -6.7}));
}

TEST(PosixConfig, NegativeNumberWithoutArg)
{
    struct Cfg : public Config{
        PARAM(param, int);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '-2'"});
        });
}

TEST(PosixConfig, ArgsDelimiter)
{
    struct Cfg : public Config{
        PARAM(param, int);
        PARAM(optionalParam, int)(0);
        ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.read({"-p", "11", "0", "--", "1", "-o", "2"});
    EXPECT_EQ(cfg.param, 11);
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-o", "2"}));
}

TEST(PosixConfig, ArgsDelimiterFront)
{
    struct Cfg : public Config{
        PARAM(optionalParam, int)(0);
        ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.read({"--", "0", "1", "-o", "1", "2"});
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-o", "1", "2"}));
}

TEST(PosixConfig, ArgsDelimiterBack)
{
    struct Cfg : public Config{
        PARAM(optionalParam, int)(0);
        ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.read({"-o","1", "0", "1", "2", "--"});
    EXPECT_EQ(cfg.optionalParam, 1);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "2"}));
}

TEST(PosixConfig, PascalNames)
{
    struct PascalConfig : public Config{
        PARAM(RequiredParam, std::string);
        PARAM(OptionalParam, std::string)("defaultValue");
        PARAM(IntParamOptional, std::optional<int>)();
        PARAMLIST(ListOfParam, std::string);
        PARAMLIST(MyListOfParamOptional, int)(std::vector<int>{99,100});
        FLAG(Flag);
        ARG(Arg, double);
        ARGLIST(ArgList, float);
    };
    auto cfg = PascalConfig{};
    cfg.read({"-r","FOO", "-o", "BAR", "-i","9", "-l", "zero", "-l","one",
              "-m","1", "-m","2", "-f", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.RequiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.OptionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.IntParamOptional, 9);
    EXPECT_EQ(cfg.ListOfParam, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.MyListOfParamOptional, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.Flag, true);
    EXPECT_EQ(cfg.Arg, 4.2);
    EXPECT_EQ(cfg.ArgList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(PosixConfig, CustomNamesMissingParam)
{
    struct TestConfig : public Config{
        PARAM(param, double) << cmdlime::Name{"P"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-P' is missing."});
        });
}

TEST(PosixConfig, CustomNamesMissingParamList)
{
    struct TestConfig : public Config{
        PARAM(param, double) << cmdlime::Name{"P"};
        PARAMLIST(paramList, float) << cmdlime::Name{"L"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-P1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-L' is missing."});
        });
}

TEST(PosixConfig, CustomNamesMissingArg)
{
    struct TestConfig : public Config{
        ARG(arg, double) << cmdlime::Name{"a"};
        ARGLIST(argList, float) << cmdlime::Name{"A"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'a' is missing."});
        });
}

TEST(PosixConfig, CustomNamesMissingArgList)
{
    struct TestConfig : public Config{
        ARG(arg, double) << cmdlime::Name{"a"};
        ARGLIST(argList, float) << cmdlime::Name{"A"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"1.0"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'A' is missing."});
        });
}

TEST(PosixConfig, ConfigErrorRepeatingParamNames)
{
    struct TestConfig : public Config{
        PARAM(Param, double)();
        PARAM(param, int)();
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.read({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'p' is already used."});
        });
}

TEST(PosixConfig, UsageInfo)
{
    auto cfg = FullConfig{};
    auto expectedInfo = std::string{
    "Usage: testproc <arg> -r <string> -L <string>... "
    "[-o <string>] [-i <int>] [-O <int>...] [-f] <arg-list...>\n"
    };
    EXPECT_EQ(cfg.usageInfo("testproc"), expectedInfo);
}

TEST(PosixConfig, DetailedUsageInfo)
{
    auto cfg = FullConfig{};
    auto expectedDetailedInfo = std::string{
    "Usage: testproc <arg> -r <string> -L <string>... [params] [flags] <arg-list...>\n"
    "Arguments:\n"
    "    <arg> (double)         \n"
    "    <arg-list> (float)     List (can be used multiple times).\n"
    "Parameters:\n"
    "   -r <string>             \n"
    "   -L <string>             List (can be used multiple times).\n"
    "   -o <string>             Optional, default: defaultValue\n"
    "   -i <int>                Optional.\n"
    "   -O <int>                List (can be used multiple times).\n"
    "                             Optional, default: {99, 100}\n"
    "Flags:\n"
    "   -f                      \n"};
    EXPECT_EQ(cfg.usageInfoDetailed("testproc"), expectedDetailedInfo);
}

}
