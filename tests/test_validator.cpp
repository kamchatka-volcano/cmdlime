#include <gtest/gtest.h>
#include <cmdlime/gnuconfig.h>
#include "assert_exception.h"
#include <optional>

namespace test_validator{


using Config = cmdlime::GNUConfig;

struct EnsureNotShorterThan{
    EnsureNotShorterThan(int size)
            : minSize_(size)
    {}

    template <typename T>
    void operator()(const T& val)
    {
        if (static_cast<int>(val.size()) < minSize_)
            throw cmdlime::ValidationError{"size can't be less than 2."};
    }

private:
    int minSize_;
};

struct EnsurePositive{
    template <typename T>
    void operator()(const T& val)
    {
        if (val < 0)
            throw cmdlime::ValidationError{"value can't be negative."};
    }
};

struct NestedSubcommandConfig: public Config{
    CMDLIME_PARAM(param, std::string) << EnsureNotShorterThan{2};
};

struct SubcommandConfig: public Config{
    CMDLIME_PARAM(requiredParam, std::string) << EnsureNotShorterThan{2};
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue") << EnsureNotShorterThan{2};
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i") << EnsurePositive{};
    CMDLIME_PARAMLIST(paramList, std::string) << cmdlime::ShortName("L") << EnsureNotShorterThan{2};
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100}) << cmdlime::ShortName("O") << EnsureNotShorterThan{2};
    CMDLIME_FLAG(flag);
    CMDLIME_ARG(arg, double) << EnsurePositive{};
    CMDLIME_ARGLIST(argList, float) << EnsureNotShorterThan{2};
    CMDLIME_COMMAND(nested, NestedSubcommandConfig)
        << [](auto& command) {
            if (command && command->param.size() >= 5)
                throw cmdlime::ValidationError{"command param must have a length shorter than 5."};
        };
};


struct FullConfig : public Config{
    CMDLIME_PARAM(requiredParam, std::string)
    << [](auto param) {
        if (param.size() < 2)
            throw cmdlime::ValidationError{"length can't be less than 2."};
    };
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue")
    << [](auto param) {
        if (param.size() < 2)
            throw cmdlime::ValidationError{"length can't be less than 2."};
    };
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i")
        << [](auto param) {
            if (param && param < 0)
                throw cmdlime::ValidationError{"value can't be negative."};
        };

    CMDLIME_PARAMLIST(paramList, std::string) << cmdlime::ShortName("L")
        << [](auto param) {
            if (param.size() < 2)
                throw cmdlime::ValidationError{"size can't be less than 2."};
        };
    CMDLIME_PARAMLIST(optionalParamList, int)(std::vector<int>{99, 100}) << cmdlime::ShortName("O")
            << [](auto param) {
                if (param.size() < 2)
                    throw cmdlime::ValidationError{"size can't be less than 2."};
            };
    CMDLIME_ARG(arg, double)
        << [](auto param) {
            if (param < 0)
                throw cmdlime::ValidationError{"value can't be negative."};
        };
    CMDLIME_ARGLIST(argList, float)
        << [](auto param) {
            if (param.size() < 2)
                throw cmdlime::ValidationError{"size can't be less than 2."};
        };
    CMDLIME_COMMAND(command, SubcommandConfig)
        << [](auto& command){
            if (command && command->requiredParam.size() >= 5)
                throw cmdlime::ValidationError{"command required param must have a length shorter than 5."};
        };
    CMDLIME_SUBCOMMAND(subcommand, SubcommandConfig)
        << [](auto& command) {
            if (command && command->requiredParam.size() >= 5)
                throw cmdlime::ValidationError{"command required param must have a length shorter than 5."};
        };
};

TEST(TestValidator, AllSet)
{
    auto cfg = FullConfig{};
    cfg.readCommandLine({"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                         "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.paramList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.arg, 4.2);
    EXPECT_EQ(cfg.argList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    EXPECT_FALSE(cfg.command.has_value());
    EXPECT_FALSE(cfg.subcommand.has_value());
}

TEST(TestValidator, InvalidParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"-r", "F", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter 'required-param' is invalid: length can't be less than 2."});
            });
}

TEST(TestValidator, InvalidOptionalParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"-r", "FOO", "-oB", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter 'optional-param' is invalid: length can't be less than 2."});
            });
}

TEST(TestValidator, InvalidOptionalIntParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"-r", "FOO", "-oBAR", "--optional-int-param", "-9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter 'optional-int-param' is invalid: value can't be negative."});
            });
}

TEST(TestValidator, InvalidParamList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero",
                                        "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter list 'param-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidOptionalParamList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter list 'optional-param-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidArg)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "-4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Argument 'arg' is invalid: value can't be negative."});
            });
}

TEST(TestValidator, InvalidArgList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "4.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Argument list 'arg-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommand)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "-r", "F00BAR", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'command' is invalid: command required param must have a length shorter than 5."});
            });
}

TEST(TestValidator, InvalidCommandParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "-r", "F", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'command's parameter 'required-param' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommandOptionalParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "-r", "FOO", "-oB", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'command's parameter 'optional-param' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommandOptionalIntParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "-r", "FOO", "-oBAR", "--optional-int-param", "-9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'command's parameter 'optional-int-param' is invalid: value can't be negative."});
            });
}

TEST(TestValidator, InvalidCommandParamList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero",
                                        "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'command's parameter list 'param-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommandOptionalParamList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'command's parameter list 'optional-param-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommandArg)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "-4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'command's argument 'arg' is invalid: value can't be negative."});
            });
}

TEST(TestValidator, InvalidCommandArgList)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L","zero", "-L", "one",
                                        "--optional-param-list=1,2", "4.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'command's argument list 'arg-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidNestedCommand)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "nested", "--param", "F00BAR"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'command's command 'nested' is invalid: command param must have a length shorter than 5."});
            });
}

TEST(TestValidator, InvalidNestedCommandParam)
{
    auto cfg = FullConfig{};
    assert_exception<cmdlime::ParsingError>(
            [&cfg]{cfg.readCommandLine({"command", "nested", "--param", "F"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'nested's parameter 'param' is invalid: size can't be less than 2."});
            });
}

}
