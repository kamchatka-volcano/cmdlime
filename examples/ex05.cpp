#include <cmdlime/config.h>
#include <iostream>

int main(int argc, char** argv)
{
    struct Cfg : public cmdlime::Config{
        ARG(zipCode, int);
        PARAM(name, std::string);
        FLAG(verbose);
        EXITFLAG(help)    << cmdlime::WithoutShortName{};
        EXITFLAG(version) << cmdlime::WithoutShortName{};
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

