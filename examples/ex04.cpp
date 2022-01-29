#include <cmdlime/config.h>
#include <iostream>

int main(int argc, char** argv)
{
    struct Cfg : public cmdlime::Config{
        CMDLIME_ARG(zipCode, int);
        CMDLIME_PARAM(name, std::string);
        CMDLIME_FLAG(verbose);
        CMDLIME_EXITFLAG(help);
        CMDLIME_EXITFLAG(version);
    } cfg;

    try{
        cfg.readCommandLine(argc, argv);
    }
    catch(const cmdlime::Error& e){
        std::cerr << e.what();
        std::cout << cfg.usageInfo("person-finder");
        return -1;
    }
    if (cfg.help){
        std::cout << cfg.usageInfoDetailed("person-finder");
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

