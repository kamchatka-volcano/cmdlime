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

