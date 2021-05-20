#include <cmdlime/config.h>
#include <iostream>

int main(int argc, char** argv)
{
    struct Cfg : public cmdlime::Config{
        ARG(zipCode, int);
        PARAM(surname, std::string)  << cmdlime::ValueName{"A-Z..."};
        PARAM(name, std::string)()   << cmdlime::Name{"first-name"};
        FLAG(verbose);
    } cfg;

    cfg.setVersionInfo("person-finder 1.0");
    auto reader = cmdlime::ConfigReader{cfg, "person-finder"};
    if (!reader.readCommandLine(argc, argv))
        return reader.exitCode();

    //At this point your config is ready to use
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode;
    return 0;
}
