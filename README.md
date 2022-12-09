<p align="center">
  <img height="128" src="doc/logo.jpg"/>  
</p>

[![build & test (clang, gcc, MSVC)](https://github.com/kamchatka-volcano/cmdlime/actions/workflows/build_and_test.yml/badge.svg?branch=master)](https://github.com/kamchatka-volcano/cmdlime/actions/workflows/build_and_test.yml)

**cmdlime** is a C++17 header-only library for command line parsing that provides a concise, declarative interface without many details to remember. See for yourself:

```C++
///examples/ex01.cpp
///
#include <cmdlime/commandlinereader.h>
#include <iostream>

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int);
    CMDLIME_PARAM(name, std::string);
    CMDLIME_FLAG(verbose);
};

int main(int argc, char** argv)
{
    auto reader = cmdlime::CommandLineReader{"person-finder"};
    auto cfg = reader.read<Cfg>(argc, argv);

    //At this point your config is ready to use
    std::cout << "Looking for person " << cfg.name << " in the region with zip code: " << cfg.zipCode;
    return 0;
}
```

The default configuration follows the GNU command line options convention, so you can run the program like this:
```console
kamchatka-volcano@home:~$ ./person-finder 684007 --name John --verbose
Looking for person John in the region with zip code: 684007
```

Please note that in the example above, `--name` is a parameter, `--verbose` is a flag, and `684007` is an argument. This naming convention is used throughout this document and in the **cmdlime** interface.


## Table of Contents
*    [Usage](#usage)
     * [Declaring the config structure](#declaring-the-config-structure)
     * [Avoiding macros](#avoiding-macros)
     * [Using CommandLineReader::exec()](#using-commandlinereaderexec) 
     * [Custom names](#custom-names)
     * [Auto-generated usage info](#auto-generated-usage-info)
     * [Supported formats](#supported-formats)
          *    [GNU](#gnu)
          *    [POSIX](#posix)
          *    [X11](#x11)
          *    [Simple format](#simple-format)
     * [Using custom types](#using-custom-types)
     * [Using subcommands](#using-subcommands)
     * [Using validators](#using-validators)
*    [Installation](#installation)
*    [Running tests](#running-tests)
*    [Building examples](#building-examples)
*    [License](#license)

## Usage


### Declaring the config structure

To use **cmdlime**, you need to create a structure with fields corresponding to the parameters, flags, and arguments that will be read from the command line.

To do this, subclass `cmdlime::Config` and declare fields using the following macros:

- **CMDLIME_ARG(`name`, `type`)** - creates a `type name;` config field and registers it in the parser.
Arguments are mapped to the config fields in the order of declaration. Arguments cannot have default values and must be specified in the command line.

- **CMDLIME_ARGLIST(`name`, `listType`)** - creates `listType name;` config field and registers it in the parser. `listType` can be any sequence container that supports the `emplace_back` operation; within the STL, this includes `vector`, `deque`, or `list`. A config can have only one argument list, and elements are placed into it after all other config arguments have been set, regardless of the order of declaration. The declaration form `CMDLIME_ARGLIST(name, listType)(list-initialization)` sets the default value of an argument list, making it optional and allowing it to be omitted from the command line without raising an error.

- **CMDLIME_PARAM(`name`, `type`)** - creates a `type name;` config field and registers it in the parser.
The declaration form `CMDLIME_PARAM(name, type)(default value)` sets the default value of a parameter, making it optional and allowing it to be omitted from the command line without raising an error. Parameters can also be declared optional by placing them in `cmdlime::optional` (a `std::optional`-like wrapper with a similar interface).

- **CMDLIME_PARAMLIST(`name`, `listType`)** - creates `listType name;` config field and registers it in the parser. `listType` can be any sequence container that supports the `emplace_back` operation; within the STL, this includes `vector`, `deque`, or `list`.
A parameter list can be filled by specifying it multiple times in the command line (e.g., `--param-list val1 --param-list val2`) or by passing a comma-separated value (e.g., `--param-list val1,val2`).
The declaration form `CMDLIME_PARAMLIST(name, type)(list-initialization)` sets the default value of a parameter list, making it optional and allowing it to be omitted from the command line without raising an error.
- **CMDLIME_FLAG(`name`)** - creates a `bool name;` config field and registers it in the parser.
Flags are always optional and have a default value of `false`.

- **CMDLIME_EXITFLAG(`name`)** - creates a `bool name;` config field and registers it in the parser.
If at least one exit flag is set, no parsing errors will be raised regardless of the command line's content. The other config fields will be left in an unspecified state. This is useful for flags like `--help` or `--version`, when you need to print a message and exit the program without checking the other fields.
- **CMDLIME_SUBCOMMAND(`name`, `type`)** - creates a `cmdlime::optional<type> name;` config field for a nested configuration structure and registers it in the parser. `type` must be a subclass of `cmdlime::Config`. Subcommands are always optional and have a default value of `cmdlime::optional<type>{}`.

- **CMDLIME_COMMAND(`name`, `type`)** - creates a `cmdlime::optional<type> name;` config field for a nested configuration structure and registers it in the parser. `type` must be a subclass of `cmdlime::Config`. Commands are always optional and have a default value of `cmdlime::optional<type>{}`. If a command is encountered, no parsing errors will be raised for the other config fields, and they will be left in an unspecified state.


*Note: Types used for config fields must be default constructable and copyable.*  

*Another note: You don't need to change your code style when declaring config fields - `camelCase`, `snake_case` and `PascalCase` names are supported and read from the `kebab-case` named parameters in the command line.*  

Let's alter the config for the `person-finder` program by adding a required parameter `surname` and making the `name` parameter optional:
```C++
///examples/ex02.cpp
///
struct Cfg : public cmdlime::Config{ 
    CMDLIME_ARG(zipCode, int);
    CMDLIME_PARAM(surname, std::string);
    CMDLIME_PARAM(name, std::string)();
    CMDLIME_FLAG(verbose);
};
```
Now parameter `--name` can be skipped without raising an error:
```console
kamchatka-volcano@home:~$ ./person-finder 684007 --surname Deer
Looking for person Deer in region with zip code: 684007
```

### Avoiding macros
If you have a low tolerance for macros, it's possible to register structure fields using the similarly named `cmdlime::Config`'s methods:
```c++
    struct Cfg : public cmdlime::Config{
        int zipCode      = arg<&Cfg::zipCode>();
        std::string name = param<&Cfg::name>();
        bool verbose     = flag<&Cfg::verbose>();
    };
```
Internally, these methods use the [nameof](https://github.com/Neargye/nameof) library to get config fields' names and types as strings. By default, **cmdlime** ships without it and these methods aren't available. To use them, you can enable the `CMDLIME_USE_NAMEOF` CMake variable to automatically download and configure the **nameof** library, or install it on your system yourself.  Note that on the MSVC compiler, some **nameof** features used by **cmdlime** require the C++20 standard. This is handled automatically by CMake configuration if MSVC is your default compiler, otherwise you will need to enable the C++20 standard manually.  
**nameof** relies on non-standard functionality of C++ compilers, so if you don't like it, you can use **cmdlime** without it by providing the names yourself:

```c++
    struct Cfg : public cmdlime::Config{
        int zipCode      = arg<&Cfg::zipCode>("zipCode", "int");
        std::string name = param<&Cfg::name>("name", "string");
        bool verbose     = flag<&Cfg::verbose>("verbose"); //flag are always booleans, so we don't need to specify a type's name here
    };
``` 

Config structures declared using the macros-free methods are fully compatible with all **cmdlime**'s functionality. Examples use registration with macros as it's the least verbose method.

### Using CommandLineReader::exec()

`CommandLineReader::exec()` is a helper method that hides the error handling boilerplate and adds `--help` and `--version` flags processing to your config.

The `--help` flag shows a detailed help message, which can otherwise be accessed through the `CommandLineReader::usageInfoDetailed()` method.

The `--version` flag is enabled only if version info is set in the config with the `CommandLineReader::setVersionInfo` method.

To use `CommandLineReader::exec()`, you need to provide an alternative entry point function for your program, which takes a processed config structure object and returns a result code. Let's modify `person-finder` and see how it works.

```C++
///examples/ex03.cpp
///
#include <cmdlime/commandlinereader.h>
#include <iostream>

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int);
    CMDLIME_PARAM(surname, std::string);
    CMDLIME_PARAM(name, std::string)();
    CMDLIME_FLAG(verbose);
};

int mainApp(const Cfg& cfg)
{
    //Here your config is ready to use
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode;
    return 0;
}

int main(int argc, char** argv)
{
    auto reader = cmdlime::CommandLineReader{"person-finder"};
    reader.setVersionInfo("person-finder 1.0");
    return reader.exec<Cfg>(argc, argv, mainApp);
}
```
```console
kamchatka-volcano@home:~$ ./person-finder --version
person-finder 1.0
kamchatka-volcano@home:~$ ./person-finder --help
Usage: person-finder <zip-code> --surname <string> [params] [flags] 
Arguments:
    <zip-code> (int)          
Parameters:
   -s, --surname <string>     
   -n, --name <string>        optional
Flags:
   -v, --verbose              
       --help                 show usage info and exit
       --version              show version info and exit
```

As mentioned before, `CommandLineReader::exec()` is just a helper method, so if you prefer to type a lot, it's possible to implement the same program without using it:
```C++
///examples/ex04.cpp
///
#include <cmdlime/commandlinereader.h>
#include <iostream>

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int);
    CMDLIME_PARAM(name, std::string);
    CMDLIME_FLAG(verbose);
    CMDLIME_EXITFLAG(help);
    CMDLIME_EXITFLAG(version);
};

int main(int argc, char** argv)
{
    auto reader = cmdlime::CommandLineReader{"person-finder"};
    auto cfg = Cfg{};
    try{
        cfg = reader.read<Cfg>(argc, argv);
    }
    catch(const cmdlime::Error& e){
        std::cerr << e.what();
        std::cout << reader.usageInfo<Cfg>();
        return -1;
    }
    if (cfg.help){
        std::cout << reader.usageInfoDetailed<Cfg>();
        return 0;
    }
    if (cfg.version){
        std::cout << "person-finder 1.0";
        return 0;
    }
    //At this point your config is ready to use
    std::cout << "Looking for person " << cfg.name << " in the region with zip code: " << cfg.zipCode;
    return 0;
}


```

Try to run it and...
```console
Usage: person-finder <zip-code> --name <string> [--verbose] [--help] [--version]
Flag's short name 'v' is already used.
```
you'll get this error. The thing is, the default command line format supports short names and our flags `--verbose` and `--version` ended up having the same short name `-v`. Read the next section to learn how to fix it.


### Custom names

```C++
///examples/ex05.cpp
///
struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int);
    CMDLIME_PARAM(name, std::string);
    CMDLIME_FLAG(verbose);
    CMDLIME_EXITFLAG(help)    << cmdlime::WithoutShortName{};
    CMDLIME_EXITFLAG(version) << cmdlime::WithoutShortName{};
};

```
Here's the fixed config. Turning off the short name generation for the flag `--version` resolves the name conflict. When you rely on `CommandLineReader::exec()` for handling of `--help` and `--version` flags, it creates them without short names. At this point, we should do this as well, and all following examples will be based on the version of `person-finder` program that uses `CommandLineReader::exec()`.


You can use the following objects to customize names generation:  
`cmdlime::Name{"customName"}` - overrides the command line option's name.  
`cmdlime::ShortName{"customShortName"}` - overrides the command line option's short name.  
`cmdlime::WithoutShortName{}` - removes the command line option's short name.  
`cmdlime::ValueName{}` - overrides the parameter's value name in the usage info. 

And it's time for another `person-finder`'s rewrite:
```C++
///examples/ex06.cpp
///
struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int);
    CMDLIME_PARAM(surname, std::string)  << cmdlime::ValueName{"A-Z..."};
    CMDLIME_PARAM(name, std::string)()   << cmdlime::Name{"first-name"};
    CMDLIME_FLAG(verbose);
};
```
```console
kamchatka-volcano@home:~$ ./person-finder --help
Usage: person-finder <zip-code> --surname <A-Z...> [params] [flags] 
Arguments:
    <zip-code> (int)             
Parameters:
   -s, --surname <A-Z...>     
   -n, --first-name <string>     optional
Flags:
   -v, --verbose                 
       --help                    show usage info and exit
       --version                 show version info and exit

```
### Auto-generated usage info

**cmdlime** can generate help messages with the `CommandLineReader::usageInfo()` and `CommandLineReader::usageInfoDetailed()` methods. The former is a compact version that can be shown with error messages, while the latter is a detailed version that is printed when the `--help` flag is set.

You can add more information to the detailed usage info by setting parameter descriptions:
```C++
///examples/ex07.cpp
///
struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int)              << "zip code of the searched region";
    CMDLIME_PARAM(surname, std::string)    << "surname of the person to find"       << cmdlime::ValueName{"A-Z..."};
    CMDLIME_PARAM(name, std::string)()     << "name of the person to find"          << cmdlime::Name{"first-name"};
    CMDLIME_FLAG(verbose)                  << "adds more information to the output";
};

```
```console
kamchatka-volcano@home:~$ ./person-finder --help
Usage: person-finder <zip-code> --surname <A-Z...> [params] [flags] 
Arguments:
    <zip-code> (int)             zip code of the searched region
Parameters:
   -s, --surname <A-Z...>     surname of the person to find
   -n, --first-name <string>     name of the person to find
                                   (optional)
Flags:
   -v, --verbose                 adds more information to the output
       --help                    show usage info and exit
       --version                 show version info and exit
```

If you don't like auto-generated usage info message you can set your own with `CommandLineReader::setUsageInfo()` and `CommandLineReader::setUsageInfoDetailed()`

### Supported formats

**cmdlime** supports several command line naming conventions and unlike other parsing libraries it enforces them strictly, so you can't mix different formats together.

All formats support the `--` argument delimiter. After encountering it, all command line options are treated as arguments, even if they start with hyphens.

#### GNU

All names are in `kebab-case`.  
Parameters and flags prefix: `--`  
Short names are supported. Short names prefix: `-`  
Parameters usage: `--parameter value`, `--parameter=value`, `-p value` or `-pvalue`  
Flags usage: `--flag`, `-f`  
Flags in short form can be "glued" together: `-abc` or with one parameter: `-fp value`

This is the default command line format used by **cmdlime**. You can choose this format explicitly by using the `CommandLineReader<cmdlime::Format::GNU>` specialization or its alias `GNUCommandLineReader`.

```C++
///examples/ex08.cpp
///
int main(int argc, char** argv)
{
    auto reader = cmdlime::GNUCommandLineReader{"person-finder"};
    reader.setVersionInfo("person-finder 1.0");
    return reader.exec<Cfg>(argc, argv, mainApp);
}
```

```console
kamchatka-volcano@home:~$ ./person-finder --help
Usage: person-finder <zip-code> --surname <string> [params] [flags] 
Arguments:
    <zip-code> (int)          zip code of the searched region
Parameters:
   -s, --surname <string>     surname of the person to find
   -n, --name <string>        name of the person to find
                                (optional)
Flags:
   -v, --verbose              adds more information to the output
       --help                 show usage info and exit
       --version              show version info and exit
```

#### POSIX

All names consist of a single alphanumeric character.  
Parameters and flags prefix: `-`  
Short names aren't supported (the default names are already short enough).  
Parameters usage: `-p value` or `-pvalue`  
Flags usage: `-f`  
Flags in short form can be "glued" together: `-abc` or with one parameter: `-fp value`

Parameters and flags must precede the arguments. Other than that, this format is a subset of the GNU format.

You can choose this format by using the `CommandLineReader<cmdlime::Format::POSIX>` specialization or its alias `POSIXCommandLineReader`.
```C++
///examples/ex09.cpp
///
int main(int argc, char** argv)
{
    auto reader = cmdlime::POSIXCommandLineReader{"person-finder"};
    reader.setVersionInfo("person-finder 1.0");
    return reader.exec<Cfg>(argc, argv, mainApp);
}

```
```console
kamchatka-volcano@home:~$ ./person-finder -h
Usage: person-finder <zip-code> -s <string> [params] [flags] 
Arguments:
    <zip-code> (int)     zip code of the searched region
Parameters:
   -s <string>           surname of the person to find
   -n <string>           name of the person to find
                           (optional)
Flags:
   -V                    adds more information to the output
   -h                    show usage info and exit
   -v                    show version info and exit
```

#### X11

All names are in `lowercase`.  
Parameters and flags prefix: `-`  
Short names aren't supported.  
Parameters usage: `-parameter value`  
Flags usage: `-flag`

You can choose this format by using the `CommandLineReader<cmdlime::Format::X11>` specialization or its alias `X11CommandLineReader`.

```C++
///examples/ex10.cpp
///
int main(int argc, char** argv)
{
    auto reader = cmdlime::X11CommandLineReader{"person-finder"};
    reader.setVersionInfo("person-finder 1.0");
    return reader.exec<Cfg>(argc, argv, mainApp);
}
```
```console
kamchatka-volcano@home:~$ ./person-finder -help
Usage: person-finder <zipcode> -surname <string> [params] [flags] 
Arguments:
    <zipcode> (int)      zip code of the searched region
Parameters:
   -surname <string>     surname of the person to find
   -name <string>        name of the person to find
                           (optional)
Flags:
   -verbose              adds more information to the output
   -help                 show usage info and exit
   -version              show version info and exit
```

#### Simple format

This format is intended for development purposes of **cmdlime**, as it's the easiest one to parse. As a result, **cmdlime** unit tests are probably the only software that uses it.

All names are in `camelCase`.  
Parameters prefix: `-`  
Flags prefix: `--`  
Short names aren't supported.  
Parameters usage: `-parameter=value`   
Flags usage: `--flag`

You can choose this format by using the `CommandLineReader<cmdlime::Format::Simple>` specialization or its alias `SimpleCommandLineReader`.


```C++
///examples/ex11.cpp
///
int main(int argc, char** argv)
{
    auto reader = cmdlime::SimpleCommandLineReader{"person-finder"};
    reader.setVersionInfo("person-finder 1.0");
    return reader.exec<Cfg>(argc, argv, mainApp);
}
```
```console
kamchatka-volcano@home:~$ ./person-finder --help
Usage: person-finder <zipCode> -surname=<string> [params] [flags] 
Arguments:
    <zipCode> (int)      zip code of the searched region
Parameters:
   -surname=<string>     surname of the person to find
   -name=<string>        name of the person to find
                           (optional)
Flags:
  --verbose              adds more information to the output
  --help                 show usage info and exit
  --version              show version info and exit
```

### Using custom types
To use custom types in the config, you need to add a specialization of the `cmdlime::StringConverter` struct and implement its static methods `toString and fromString`.

For example, let's add a coordinate parameter `--coord` to the `person-finder` program.

```C++
///examples/ex12.cpp
///
#include <cmdlime/commandlinereader.h>
#include <iostream>

struct Coord{
    double lat;
    double lon;
};

namespace cmdlime{
template<>
struct StringConverter<Coord>{
    static std::optional<std::string> toString(const Coord& coord)
    {
        auto stream = std::stringstream{};
        stream << coord.lat << "-" << coord.lon;
        return stream.str();
    }

    static std::optional<Coord> fromString(const std::string& data)
    {
        auto delimPos = data.find('-');
        if (delimPos == std::string::npos)
            return {};
        auto coord = Coord{};
        coord.lat = std::stod(data.substr(0, delimPos));
        coord.lon = std::stod(data.substr(delimPos + 1, data.size() - delimPos - 1));
        return coord;
    }
};
}

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int)              << "zip code of the searched region";
    CMDLIME_PARAM(surname, std::string)    << "surname of the person to find";
    CMDLIME_PARAM(name, std::string)()     << "name of the person to find";
    CMDLIME_PARAM(coord, Coord)            << "possible location";
    CMDLIME_FLAG(verbose)                  << "adds more information to the output";
};

int mainApp(const Cfg& cfg)
{
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode << std::endl;
    std::cout << "Possible location:" << cfg.coord.lat << " " << cfg.coord.lon;
    return 0;
}

int main(int argc, char** argv)
{
    auto reader = cmdlime::CommandLineReader{"person-finder"};
    reader.setVersionInfo("person-finder 1.0");
    return reader.exec<Cfg>(argc, argv, mainApp);
}
```
```console
kamchatka-volcano@home:~$ ./person-finder 684007 --surname Deer --coord 53.0-157.25
Looking for person  Deer in the region with zip code: 684007
Possible location:53 157.25
```

### Using subcommands

With **cmdlime**, it's possible to place a config structure inside another config field by creating a subcommand. Subcommands are specified in the command line by their full name, and all following parameters are used to fill the subcommand's structure instead of the main one.  
Let's enhance `person-finder` program by adding a result recording mode.
```C++
///examples/ex13.cpp
///
#include <cmdlime/commandlinereader.h>

struct RecordCfg: public cmdlime::Config{
    CMDLIME_PARAM(file, std::string)() << "save result to file";
    CMDLIME_PARAM(db, std::string)()   << "save result to database";
    CMDLIME_FLAG(detailed)             << "adds more information to the result" << cmdlime::WithoutShortName{};
};

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int)               << "zip code of the searched region";
    CMDLIME_PARAM(surname, std::string)     << "surname of the person to find";
    CMDLIME_PARAM(name, std::string)()      << "name of the person to find";
    CMDLIME_FLAG(verbose)                   << "adds more information to the output";
    CMDLIME_SUBCOMMAND(record, RecordCfg)   << "record search result";
};

int mainApp(const Cfg& cfg)
{
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode << std::endl;
    if (cfg.record.has_value())
        std::cout << "Record settings: " << "file:" << cfg.record->file << " db:" << cfg.record->db << " detailed:" << cfg.record->detailed << std::endl;
    return 0;
}

int main(int argc, char** argv)
{
    return cmdlime::CommandLineReader{"person-finder"}.exec<Cfg>(argc, argv, mainApp);
}

```
Now, `person-finder` can be launched like this:

```console
kamchatka-volcano@home:~$ ./person-finder 684007 --surname Deer record --file res.txt --detailed
Looking for person  Deer in the region with zip code: 684007
Record settings: file:res.txt db: detailed:1
```

Note that all required config fields, such as the `zipCode` positional argument and the `surname` parameter, must still be specified. However, some subcommands don't need those parameters. For example, imagine that the `person-finder` program has a search history mode that doesn't require them and can be launched like this: `./person-finder history` without raising a parsing error.

This can be easily achieved by registering history as a command instead of a subcommand. The main difference is that, while a command is also stored in the main config's field, logically it's an alternative configuration, not a part of the original one. When a command is present in the command line, other config fields aren't read at all and are left in an unspecified state.


Let's see how it works:

```C++
///examples/ex14.cpp
///
#include <cmdlime/commandlinereader.h>

struct RecordCfg: public cmdlime::Config{
    CMDLIME_PARAM(file, std::string)() << "save result to file";
    CMDLIME_PARAM(db, std::string)()   << "save result to database";
    CMDLIME_FLAG(detailed)             << "hide search results" << cmdlime::WithoutShortName{};
};

struct HistoryCfg: public cmdlime::Config{
    CMDLIME_PARAM(surname, std::string)() << "filter search queries by surname";
    CMDLIME_FLAG(noResults)               << "hide search results";
};

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int)             << "zip code of the searched region";
    CMDLIME_PARAM(surname, std::string)   << "surname of the person to find";
    CMDLIME_PARAM(name, std::string)()    << "name of the person to find";
    CMDLIME_FLAG(verbose)                 << "adds more information to the output";
    CMDLIME_SUBCOMMAND(record, RecordCfg) << "record search result";
    CMDLIME_COMMAND(history, HistoryCfg)  << "show search history";
};

int mainApp(const Cfg& cfg)
{
    if (cfg.history.has_value()){
        std::cout << "Preparing search history with surname filter:" << cfg.history->surname << std::endl;
        return 0;
    }

    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode << std::endl;
    if (cfg.record.has_value())
        std::cout << "Record settings: " << "file:" << cfg.record->file << " db:" << cfg.record->db << " detailed:" << cfg.record->detailed << std::endl;

    return 0;
}

int main(int argc, char** argv)
{
    return cmdlime::CommandLineReader{"person-finder"}.exec<Cfg>(argc, argv, mainApp);
}

```

```console
kamchatka-volcano@home:~$ ./person-finder history --surname Doe
Preparing search history with surname filter:Doe
```

As you can see, a config structure can have multiple commands, but only one can be specified for each config.

### Using validators
Processed command line options can be validated by registering constraint checking functions or callable objects. The signature must be compatible with `void (const T&)` where `T` is the type of the validated config structure field. If an option's value is invalid, a validator is required to throw an exception of type `cmdlime::ValidationError`:

```c++
struct Cfg : cmdlime::Config{
    CMDLIME_PARAM(number, int) 
        << [](int paramValue){
            if (paramValue < 0)
                throw cmdlime::ValidationError{"value can't be negative."};
        };
};
```

Let's improve `person-finder` by checking that either `file` or `db` parameter of the `record` subcommand is set and all names contain only alphabet characters:
```c++
///examples/ex15.cpp
///
#include <cmdlime/commandlinereader.h>
#include <algorithm>

struct EnsureAlpha{
    void operator()(const std::string& name)
    {
        if (!std::all_of(std::begin(name), std::end(name),
                         [](auto ch){
                             return std::isalpha(static_cast<int>(ch));
                         }))
            throw cmdlime::ValidationError{"value must contain alphabet characters only."};
    }
};

struct RecordCfg: public cmdlime::Config{
    CMDLIME_PARAM(file, std::string)() << "save result to file";
    CMDLIME_PARAM(db, std::string)()   << "save result to database";
    CMDLIME_FLAG(detailed)             << "hide search results" << cmdlime::WithoutShortName{};
};

struct HistoryCfg: public cmdlime::Config{
    CMDLIME_PARAM(surname, std::string)() << "filter search queries by surname" << EnsureAlpha{};
    CMDLIME_FLAG(noResults)               << "hide search results";
};

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int)             << "zip code of the searched region";
    CMDLIME_PARAM(surname, std::string)   << "surname of the person to find" << EnsureAlpha{};
    CMDLIME_PARAM(name, std::string)()    << "name of the person to find" 	 << EnsureAlpha{};
    CMDLIME_FLAG(verbose)                 << "adds more information to the output";
    CMDLIME_SUBCOMMAND(record, RecordCfg) << "record search result"
                                          << [](auto& record){
                                              if (record && record->file.empty() && record->db.empty())
                                                  throw cmdlime::ValidationError{"file or db paremeter must be provided."};
                                              else
                                                  throw std::runtime_error{"ERROR"};
                                          };
    CMDLIME_COMMAND(history, HistoryCfg)  << "show search history";
};

int mainApp(const Cfg& cfg)
{
    if (cfg.history.has_value()){
        std::cout << "Preparing search history with surname filter:" << cfg.history->surname << std::endl;
        return 0;
    }

    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode << std::endl;
    if (cfg.record.has_value())
        std::cout << "Record settings: " << "file:" << cfg.record->file << " db:" << cfg.record->db << " detailed:" << cfg.record->detailed << std::endl;

    return 0;
}

int main(int argc, char** argv)
{
    return cmdlime::CommandLineReader{"person-finder"}.exec<Cfg>(argc, argv, mainApp);
}

```

Now you'll get the following error messages if you provide invalid parameters:

```console
kamchatka-volcano@home:~$ ./person-finder --surname Deer 684007 record
Subcommand 'record' is invalid: file or db paremeter must be provided.
Usage: person-finder [commands] <zip-code> --surname <string> [--name <string>] [--verbose] [--help] 
```

```console
kamchatka-volcano@home:~$ ./person-finder --surname Deer1 684007
Parameter 'surname' is invalid: value must contain alphabet characters only.
Usage: person-finder [commands] <zip-code> --surname <string> [--name <string>] [--verbose] [--help] 
```



## Installation
Download and link the library from your project's CMakeLists.txt:
```
cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(cmdlime
    GIT_REPOSITORY "https://github.com/kamchatka-volcano/cmdlime.git"
    GIT_TAG "origin/master"
)
#uncomment if you need to install cmdlime with your target
#set(INSTALL_CMDLIME ON)
FetchContent_MakeAvailable(cmdlime)

add_executable(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE cmdlime::cmdlime)
```

To install the library system-wide, use the following commands:
```
git clone https://github.com/kamchatka-volcano/cmdlime.git
cd cmdlime
cmake -S . -B build
cmake --build build
cmake --install build
```

After installation, you can use the `find_package()` command to make the installed library available inside your project:
```
find_package(cmdlime 0.10.0 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cmdlime::cmdlime)   
```



## Running tests
```
cd cmdlime
cmake -S . -B build -DENABLE_TESTS=ON
cmake --build build
cd build/tests && ctest
```

## Building examples
```
cd cmdlime
cmake -S . -B build -DENABLE_EXAMPLES=ON
cmake --build build
cd build/examples
```

## License
**cmdlime** is licensed under the [MS-PL license](/LICENSE.md)  
