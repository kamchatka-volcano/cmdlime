///examples/ex06.cpp
///
#include <cmdlime/commandlinereader.h>
#include <iostream>

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int);
    CMDLIME_PARAM(surname, std::string)  << cmdlime::ValueName{"A-Z..."};
    CMDLIME_PARAM(name, std::string)()   << cmdlime::Name{"first-name"};
    CMDLIME_FLAG(verbose);
};

int mainApp(const Cfg& cfg)
{
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode;
    return 0;
}

int main(int argc, char** argv)
{
    auto reader = cmdlime::CommandLineReader{"person-finder"};
    reader.setVersionInfo("person-finder 1.0");
    return reader.exec<Cfg>(argc, argv, mainApp);
}
