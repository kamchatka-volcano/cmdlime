#include <gtest/gtest.h>
#include <cmdlime/detail/nameutils.h>

using namespace cmdlime::detail;

TEST(NameUtils, SnakeToCamel)
{
    EXPECT_EQ(toCamelCase(""), "");
    EXPECT_EQ(toCamelCase("hello_world"), "helloWorld");
    EXPECT_EQ(toCamelCase("_hello_world_"), "helloWorld");
    EXPECT_EQ(toCamelCase("great_2_see_u"), "great2SeeU");
    EXPECT_EQ(toCamelCase("helloWorld"), "helloWorld");
    EXPECT_EQ(toCamelCase("helloWorld2"), "helloWorld2");
    EXPECT_EQ(toCamelCase("HelloWorld2"), "helloWorld2");
}

TEST(NameUtils, SnakeToKebab)
{
    EXPECT_EQ(toKebabCase(""), "");
    EXPECT_EQ(toKebabCase("hello_world"), "hello-world");
    EXPECT_EQ(toKebabCase("hello1_"), "hello1");
    EXPECT_EQ(toKebabCase("_hello"), "hello");
    EXPECT_EQ(toKebabCase("_hello1"), "hello1");
    EXPECT_EQ(toKebabCase("_hello_1_"), "hello-1");
}

TEST(NameUtils, CamelToKebab)
{
    EXPECT_EQ(toKebabCase("helloWorld"), "hello-world");
    EXPECT_EQ(toKebabCase("hello"), "hello");
    EXPECT_EQ(toKebabCase("_helloWorld_"), "hello-world");
}

TEST(NameUtils, PascalToKebab)
{
    EXPECT_EQ(toKebabCase("HelloWorld"), "hello-world");
    EXPECT_EQ(toKebabCase("Hello"), "hello");
    EXPECT_EQ(toKebabCase("_HelloWorld_"), "hello-world");
}

TEST(NameUtils, SnakeToLower)
{
    EXPECT_EQ(toLowerCase(""), "");
    EXPECT_EQ(toLowerCase("hello_world"), "helloworld");
    EXPECT_EQ(toLowerCase("_hello_world_"), "helloworld");
    EXPECT_EQ(toLowerCase("hello"), "hello");
}

TEST(NameUtils, CamelToLower)
{
    EXPECT_EQ(toLowerCase("helloWorld"), "helloworld");
    EXPECT_EQ(toLowerCase("_helloWorld_"), "helloworld");
    EXPECT_EQ(toLowerCase("hello"), "hello");
}

TEST(NameUtils, NamespaceRemoval)
{
    EXPECT_EQ(typeNameWithoutNamespace("foo::bar"), "bar");
    EXPECT_EQ(typeNameWithoutNamespace("foo"), "foo");
}

TEST(NameUtils, GetTemplateType)
{
    EXPECT_EQ(templateType("optional<foo>"), "foo");
    EXPECT_EQ(typeNameWithoutNamespace("foo"), "foo");
}
