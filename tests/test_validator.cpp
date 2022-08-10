#include <gtest/gtest.h>
#include <cmdlime/config.h>
#include <cmdlime/commandlinereader.h>
#include "assert_exception.h"
#include <optional>

namespace test_validator{

using namespace cmdlime;

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
    CMDLIME_PARAM(prm, std::string) << EnsureNotShorterThan{2};
};

struct SubcommandConfig: public Config{
    CMDLIME_PARAM(requiredParam, std::string) << EnsureNotShorterThan{2};
    CMDLIME_PARAM(optionalParam, std::string)("defaultValue") << EnsureNotShorterThan{2};
    CMDLIME_PARAM(optionalIntParam, std::optional<int>)() << cmdlime::ShortName("i") << EnsurePositive{};
    CMDLIME_PARAMLIST(prmList, std::vector<std::string>) << cmdlime::ShortName("L") << EnsureNotShorterThan{2};
    CMDLIME_PARAMLIST(optionalParamList, std::vector<int>)(std::vector<int>{99, 100}) << cmdlime::ShortName("O") << EnsureNotShorterThan{2};
    CMDLIME_FLAG(flg);
    CMDLIME_EXITFLAG(exitFlg);
    CMDLIME_ARG(argument, double) << EnsurePositive{};
    CMDLIME_ARGLIST(argumentList, std::vector<float>) << EnsureNotShorterThan{2};
    CMDLIME_COMMAND(nested, NestedSubcommandConfig)
        << [](auto& command) {
            if (command && command->prm.size() >= 5)
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

    CMDLIME_PARAMLIST(prmList, std::vector<std::string>) << cmdlime::ShortName("L")
        << [](auto param) {
            if (param.size() < 2)
                throw cmdlime::ValidationError{"size can't be less than 2."};
        };
    CMDLIME_PARAMLIST(optionalParamList, std::vector<int>)(std::vector<int>{99, 100}) << cmdlime::ShortName("O")
            << [](auto param) {
                if (param.size() < 2)
                    throw cmdlime::ValidationError{"size can't be less than 2."};
            };
    CMDLIME_ARG(argument, double)
        << [](auto param) {
            if (param < 0)
                throw cmdlime::ValidationError{"value can't be negative."};
        };
    CMDLIME_ARGLIST(argumentList, std::vector<float>)
        << [](auto param) {
            if (param.size() < 2)
                throw cmdlime::ValidationError{"size can't be less than 2."};
        };
    CMDLIME_COMMAND(cmd, SubcommandConfig)
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
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    auto cfg = reader.read<FullConfig>({"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                                           "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});
    EXPECT_EQ(cfg.requiredParam, std::string{"FOO"});
    EXPECT_EQ(cfg.optionalParam, std::string{"BAR"});
    EXPECT_EQ(cfg.optionalIntParam, 9);
    EXPECT_EQ(cfg.prmList, (std::vector<std::string>{"zero", "one"}));
    EXPECT_EQ(cfg.optionalParamList, (std::vector<int>{1, 2}));
    EXPECT_EQ(cfg.argument, 4.2);
    EXPECT_EQ(cfg.argumentList, (std::vector<float>{1.1f, 2.2f, 3.3f}));
    EXPECT_FALSE(cfg.cmd);
    EXPECT_FALSE(cfg.subcommand);
}

TEST(TestValidator, InvalidParam)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>({"-r", "F", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                                            "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter 'required-param' is invalid: length can't be less than 2."});
            });
}

TEST(TestValidator, InvalidOptionalParam)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>({"-r", "FOO", "-oB", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                                            "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter 'optional-param' is invalid: length can't be less than 2."});
            });
}

TEST(TestValidator, InvalidOptionalIntParam)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"-r", "FOO", "-oBAR", "--optional-int-param", "-9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter 'optional-int-param' is invalid: value can't be negative."});
            });
}

TEST(TestValidator, InvalidParamList)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>({"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero",
                                            "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter list 'prm-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidOptionalParamList)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Parameter list 'optional-param-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidArg)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1,2", "-4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Argument 'argument' is invalid: value can't be negative."});
            });
}

TEST(TestValidator, InvalidArgList)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1,2", "4.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Argument list 'argument-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommand)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"cmd", "-r", "F00BAR", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'cmd' is invalid: command required param must have a length shorter than 5."});
            });
}

TEST(TestValidator, InvalidCommandExitFlag)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    auto cfg = reader.read<FullConfig>({"cmd", "--exit-flg"});
    ASSERT_TRUE(cfg.cmd);
    EXPECT_EQ(cfg.cmd->exitFlg, true);
}

TEST(TestValidator, InvalidCommandParam)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"cmd", "-r", "F", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'cmd's parameter 'required-param' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommandOptionalParam)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"cmd", "-r", "FOO", "-oB", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'cmd's parameter 'optional-param' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommandOptionalIntParam)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"cmd", "-r", "FOO", "-oBAR", "--optional-int-param", "-9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'cmd's parameter 'optional-int-param' is invalid: value can't be negative."});
            });
}

TEST(TestValidator, InvalidCommandParamList)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>({"cmd", "-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero",
                                            "--optional-param-list=1,2", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'cmd's parameter list 'prm-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommandOptionalParamList)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"cmd", "-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1", "4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'cmd's parameter list 'optional-param-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidCommandArg)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"cmd", "-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1,2", "-4.2", "1.1", "2.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'cmd's argument 'argument' is invalid: value can't be negative."});
            });
}

TEST(TestValidator, InvalidCommandArgList)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{
                reader.read<FullConfig>(
                        {"cmd", "-r", "FOO", "-oBAR", "--optional-int-param", "9", "-L", "zero", "-L", "one",
                         "--optional-param-list=1,2", "4.2", "3.3"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'cmd's argument list 'argument-list' is invalid: size can't be less than 2."});
            });
}

TEST(TestValidator, InvalidNestedCommand)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{ reader.read<FullConfig>({"cmd", "nested", "--prm", "F00BAR"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'cmd's command 'nested' is invalid: command param must have a length shorter than 5."});
            });
}

TEST(TestValidator, InvalidNestedCommandParam)
{
    auto reader = cmdlime::CommandLineReader<cmdlime::Format::GNU>{};
    assert_exception<cmdlime::ParsingError>(
            [&]{ reader.read<FullConfig>({"cmd", "nested", "--prm", "F"});},
            [](const cmdlime::ParsingError& error){
                EXPECT_EQ(std::string{error.what()}, std::string{"Command 'nested's parameter 'prm' is invalid: size can't be less than 2."});
            });
}

}
