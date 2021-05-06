#include <gtest/gtest.h>
#include <cmdlime/config.h>
#include "assert_exception.h"
#include <optional>

using Config = cmdlime::Config;

struct FullConfig : public Config{
    PARAM(requiredParam, std::string);
    PARAM(optionalParam, std::string)("defaultValue");
    PARAM(optionalIntParam, std::optional<int>)();
    FLAG(flag);
    ARG(arg, double);
    ARGLIST(argList, float);
};

TEST(DefaultConfig, AllSet)
{
    auto cfg = FullConfig{};
    cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flag", "4.2", "1.1", "2.2f", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.flag, true);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(DefaultConfig, MissingOptionals)
{
    auto cfg = FullConfig{};
    cfg.read({"-requiredParam=FOO", "4.2", "1.1", "2.2f", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

struct FullConfigWithOptionalArgList : public Config{
    PARAM(requiredParam, std::string);
    PARAM(optionalParam, std::string)({"defaultValue"});
    PARAM(optionalIntParam, std::optional<int>)();
    FLAG(flag);
    ARG(arg, double);
    ARGLIST(argList, float)();
};

TEST(DefaultConfig, MissingOptionalArgList)
{
    auto cfg = FullConfigWithOptionalArgList{};
    cfg.read({"-requiredParam=FOO", "4.2"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"defaultValue"});
    EXPECT_EQ(cfg.optionalIntParam.has_value(), false);
    EXPECT_EQ(cfg.flag, false);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList.empty(), true);
}

TEST(DefaultConfig, MissingRequiredParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-optionalParam=FOO", "4.2", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-requiredParam' is missing."});
        });
}

TEST(DefaultConfig, MissingArg)
{
    auto cfg = FullConfigWithOptionalArgList{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flag"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'arg' is missing."});
        });
}

TEST(DefaultConfig, MissingArgList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flag", "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Required catch-all arguments list 'argList' is missing."});
        });
}

TEST(DefaultConfig, UnexpectedParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-testParam=TEST", "4.2", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter '-testParam'"});
        });
}

TEST(DefaultConfig, UnexpectedFlag)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "--testFlag","4.2", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown flag '--testFlag'"});
        });
}

TEST(DefaultConfig, UnexpectedArg)
{
    struct Cfg : public Config{
        PARAM(param, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-param=FOO", "4.2", "1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown positional argument '4.2'"});
        });

}

TEST(DefaultConfig, WrongParamType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=nine", "--flag", "4.2", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter 'optionalIntParam' value from 'nine'"});
        });
}

TEST(DefaultConfig, WrongArgType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flag", "fortytwo", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'arg' value from 'fortytwo'"});
        });
}

TEST(DefaultConfig, WrongArgListElementType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "--flag", "4.2", "1.1", "2.2f", "three"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument list 'argList' value from 'three'"});
        });
}

TEST(DefaultConfig, ParamEmptyValue)
{
    struct Cfg : public Config{
        PARAM(param, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-param="});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-param' value can't be empty"});
        });
}

TEST(DefaultConfig, ArgEmptyValue)
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

TEST(DefaultConfig, ArgListEmptyValue)
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

TEST(DefaultConfig, ValuesWithWhitespace)
{
    struct Cfg : public Config{
        PARAM(param, std::string);
        ARG(arg, std::string);
        ARGLIST(argList, std::string);
    };
    auto cfg = Cfg{};
    cfg.read({"-param=Hello world", "foo bar", "forty two"});
    EXPECT_EQ(cfg.param, "Hello world");
    EXPECT_EQ(cfg.arg, "foo bar");
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"forty two"}));
}

TEST(DefaultConfig, ParamWrongFormat)
{
    struct Cfg : public Config{
        PARAM(param, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-param:2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Wrong parameter format: -param:2. Parameter must have a form of -name=value"});
        });
}

TEST(DefaultConfig, NegativeNumberToArg)
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

TEST(DefaultConfig, NegativeNumberWithoutArg)
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
