///examples/ex05.cpp
///
#include <cmdlime/commandlinereader.h>
#include <iostream>

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int);
    CMDLIME_PARAM(name, std::string);
    CMDLIME_FLAG(verbose);
    CMDLIME_EXITFLAG(help)    << cmdlime::WithoutShortName{};
    CMDLIME_EXITFLAG(version) << cmdlime::WithoutShortName{};
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

