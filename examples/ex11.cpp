#include <cmdlime/simpleconfig.h>
#include <iostream>

int main(int argc, char** argv)
{
    struct Cfg : public cmdlime::SimpleConfig{
        CMDLIME_ARG(zipCode, int)              << "zip code of the searched region";
        CMDLIME_PARAM(surname, std::string)    << "surname of the person to find";
        CMDLIME_PARAM(name, std::string)()     << "name of the person to find";
        CMDLIME_FLAG(verbose)                  << "adds more information to the output";
    } cfg;


    cfg.setVersionInfo("person-finder 1.0");
    auto reader = cmdlime::ConfigReader{cfg, "person-finder"};
    if (!reader.readCommandLine(argc, argv))
        return reader.exitCode();

    //At this point your config is ready to use
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode;
    return 0;
}

