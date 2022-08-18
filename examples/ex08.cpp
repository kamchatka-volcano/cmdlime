///examples/ex08.cpp
///
#include <cmdlime/commandlinereader.h>
#include <iostream>

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int)              << "zip code of the searched region";
    CMDLIME_PARAM(surname, std::string)    << "surname of the person to find";
    CMDLIME_PARAM(name, std::string)()     << "name of the person to find";
    CMDLIME_FLAG(verbose)                  << "adds more information to the output";
};

int mainApp(const Cfg& cfg)
{
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode;
    return 0;
}

int main(int argc, char** argv)
{
    auto reader = cmdlime::GNUCommandLineReader{"person-finder"};
    reader.setVersionInfo("person-finder 1.0");
    return reader.exec<Cfg>(argc, argv, mainApp);
}

