#include <gtest/gtest.h>
#include <cmdlime/config.h>
#include "assert_exception.h"
#include <optional>

using Config = cmdlime::Config;

struct FullConfig : public Config{
    PARAM(requiredParam, std::string);
    PARAM(optionalParam, std::string)("defaultValue");
    PARAM(optionalIntParam, std::optional<int>)();
    PARAMLIST(paramList, std::string);
    PARAMLIST(optionalParamList, int)(std::vector<int>{99,100});
    FLAG(flag);
    ARG(arg, double);
    ARGLIST(argList, float);
};

TEST(DefaultConfig, AllSet)
{
    auto cfg = FullConfig{};
    cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
              "-optionalParamList=1", "-optionalParamList=2", "--flag", "4.2", "1.1", "2.2f", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flag, true);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(DefaultConfig, MissingOptionals)
{
    auto cfg = FullConfig{};
    cfg.read({"-requiredParam=FOO", "-paramList=zero", "4.2", "1.1", "2.2f", "3.3"});
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
    PARAM(optionalIntParam, std::optional<int>)();
    FLAG(flag);
    ARG(arg, double);
    ARGLIST(argList, float)({1.f, 2.f});
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
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.f, 2.f}));
}

TEST(DefaultConfig, MissingParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-optionalParam=FOO","-paramList=zero", "4.2", "1.1", "2.2f", "3.3"});},
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
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9","-paramList=zero","--flag", "4.2"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'argList' is missing."});
        });
}

TEST(DefaultConfig, MissingParamList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9","--flag", "4.2", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-paramList' is missing."});
        });
}

TEST(DefaultConfig, UnexpectedParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-testParam=TEST","-paramList=zero", "4.2", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Encountered unknown parameter '-testParam'"});
        });
}

TEST(DefaultConfig, UnexpectedFlag)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "--testFlag", "-paramList=zero", "4.2", "1.1", "2.2f", "3.3"});},
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
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=nine", "-paramList=zero", "--flag", "4.2", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set parameter 'optionalIntParam' value from 'nine'"});
        });
}

TEST(DefaultConfig, WrongParamListElementType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-paramList=zero", "-optionalParamList=not-int", "--flag", "4.2", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set param 'optionalParamList' value from 'not-int'"});
        });
}

TEST(DefaultConfig, WrongArgType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9","-paramList=zero", "--flag", "fortytwo", "1.1", "2.2f", "3.3"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Couldn't set argument 'arg' value from 'fortytwo'"});
        });
}

TEST(DefaultConfig, WrongArgListElementType)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "--flag", "4.2", "1.1", "2.2f", "three"});},
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

TEST(DefaultConfig, ParamListEmptyValue)
{
    struct Cfg : public Config{
        PARAMLIST(params, std::string);
    };
    auto cfg = Cfg{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-params=", ""});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-params' value can't be empty"});
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
        PARAMLIST(paramList, std::string);
        ARG(arg, std::string);
        ARGLIST(argList, std::string);
    };
    auto cfg = Cfg{};
    cfg.read({"-param=Hello world", "-paramList=foo bar", "foo bar", "forty two"});
    EXPECT_EQ(cfg.param, "Hello world");
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"foo bar"}));
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

TEST(DefaultConfig, ArgsDelimiter)
{
    struct Cfg : public Config{
        PARAM(param, int);
        PARAM(optionalParam, int)(0);
        ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.read({"0", "-param=11", "--", "1", "-optionalParam", "2"});
    EXPECT_EQ(cfg.param, 11);
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-optionalParam", "2"}));
}

TEST(DefaultConfig, ArgsDelimiterFront)
{
    struct Cfg : public Config{
        PARAM(optionalParam, int)(0);
        ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.read({"--", "0", "1", "-optionalParam=1", "2"});
    EXPECT_EQ(cfg.optionalParam, 0);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "-optionalParam=1", "2"}));
}

TEST(DefaultConfig, ArgsDelimiterBack)
{
    struct Cfg : public Config{
        PARAM(optionalParam, int)(0);
        ARGLIST(argList, std::string);
    };

    auto cfg = Cfg{};
    cfg.read({"0", "1", "-optionalParam=1", "2", "--"});
    EXPECT_EQ(cfg.optionalParam, 1);
    EXPECT_EQ(cfg.argList, (std::vector<std::string>{"0", "1", "2"}));
}

TEST(DefaultConfig, PascalNames)
{
    struct PascalConfig : public Config{
        PARAM(RequiredParam, std::string);
        PARAM(OptionalParam, std::string)("defaultValue");
        PARAM(OptionalIntParam, std::optional<int>)();
        PARAMLIST(ParamList, std::string);
        PARAMLIST(OptionalParamList, int)(std::vector<int>{99,100});
        FLAG(Flag);
        ARG(Arg, double);
        ARGLIST(ArgList, float);
    };
    auto cfg = PascalConfig{};    
    cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
              "-optionalParamList=1", "-optionalParamList=2", "--flag", "4.2", "1.1", "2.2f", "3.3"});
    EXPECT_EQ(cfg.RequiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.OptionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.OptionalIntParam, 9);
    EXPECT_EQ(cfg.ParamList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.OptionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.Flag, true);
    EXPECT_EQ(cfg.Arg, 4.2);
    EXPECT_EQ(cfg.ArgList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(DefaultConfig, SnakeNames)
{
    struct PascalConfig : public Config{
        PARAM(required_param, std::string);
        PARAM(optional_param, std::string)("defaultValue");
        PARAM(optional_int_param, std::optional<int>)();
        PARAMLIST(param_list, std::string);
        PARAMLIST(optional_param_list_, int)(std::vector<int>{99,100});
        FLAG(flag_);
        ARG(arg_, double);
        ARGLIST(arg_list_, float);
    };
    auto cfg = PascalConfig{};
    cfg.read({"-requiredParam=FOO", "-optionalParam=BAR", "-optionalIntParam=9", "-paramList=zero", "-paramList=one",
              "-optionalParamList=1", "-optionalParamList=2", "--flag", "4.2", "1.1", "2.2f", "3.3"});
    EXPECT_EQ(cfg.required_param, std::string{"FOO"});
    EXPECT_EQ(cfg.optional_param, std::string{"BAR"});
    EXPECT_EQ(cfg.optional_int_param, 9);
    EXPECT_EQ(cfg.param_list, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optional_param_list_, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.flag_, true);
    EXPECT_EQ(cfg.arg_, 4.2);
    EXPECT_EQ(cfg.arg_list_, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(DefaultConfig, CustomNames)
{
    struct TestConfig : public Config{
        PARAM(requiredParam, std::string) << cmdlime::Name{"customRequiredParam"};
        PARAM(optionalParam, std::string)("defaultValue") << cmdlime::Name{"customOptionalParam"};
        PARAM(optionalIntParam, std::optional<int>)() << cmdlime::Name{"customOptionalIntParam"};
        FLAG(flag) << cmdlime::Name{"customFlag"};
        ARG(arg, double) << cmdlime::Name{"customArg"};
        ARGLIST(argList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfg = TestConfig{};
    cfg.read({"-customRequiredParam=FOO", "-customOptionalParam=BAR", "-customOptionalIntParam=9", "--customFlag", "4.2", "1.1", "2.2f", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.flag, true);
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
}

TEST(DefaultConfig, CustomNamesMissingParam)
{
    struct TestConfig : public Config{
        PARAM(param, double) << cmdlime::Name{"customParam"};
        PARAMLIST(paramList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-customParam' is missing."});
        });
}

TEST(DefaultConfig, CustomNamesMissingParamList)
{
    struct TestConfig : public Config{
        PARAM(param, double) << cmdlime::Name{"customParam"};
        PARAMLIST(paramList, float) << cmdlime::Name{"customParamList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"-customParam=1"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter '-customParamList' is missing."});
        });
}

TEST(DefaultConfig, CustomNamesMissingArg)
{
    struct TestConfig : public Config{
        ARG(arg, double) << cmdlime::Name{"customArg"};
        ARGLIST(argList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Positional argument 'customArg' is missing."});
        });
}

TEST(DefaultConfig, CustomNamesMissingArgList)
{
    struct TestConfig : public Config{
        ARG(arg, double) << cmdlime::Name{"customArg"};
        ARGLIST(argList, float) << cmdlime::Name{"customArgList"};
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ParsingError>(
        [&cfg]{cfg.read({"1.0"});},
        [](const cmdlime::ParsingError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Arguments list 'customArgList' is missing."});
        });
}

TEST(DefaultConfig, ConfigErrorRepeatingNames)
{
    struct TestConfig : public Config{
        PARAM(Param, double)();
        PARAM(param, int)();
    };
    auto cfg = TestConfig{};
    assert_exception<cmdlime::ConfigError>(
        [&cfg]{cfg.read({});},
        [](const cmdlime::ConfigError& error){
            EXPECT_EQ(std::string{error.what()}, std::string{"Parameter's name 'param' is already used."});
        });
}



