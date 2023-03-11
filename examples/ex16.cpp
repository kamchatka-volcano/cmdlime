///examples/ex16.cpp
///
#include <cmdlime/commandlinereader.h>
#include <algorithm>

struct Cfg : public cmdlime::Config {
    CMDLIME_ARG(zipCode, int) << "zip code of the searched region";
    CMDLIME_PARAM(surname, std::string) << "surname of the person to find";
    CMDLIME_PARAM(name, std::string)() << "name of the person to find";
    CMDLIME_FLAG(verbose) << "adds more information to the output";
};

namespace cmdlime {
template<>
struct PostProcessor<Cfg> {
    void operator()(Cfg& cfg)
    {
        if (cfg.name.empty())
            std::transform(
                    cfg.surname.begin(),
                    cfg.surname.end(),
                    cfg.surname.begin(),
                    [](const auto& ch)
                    {
                        return sfun::toupper(ch);
                    });
    }
};
} //namespace cmdlime

int mainApp(const Cfg& cfg)
{
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname
              << " in the region with zip code: " << cfg.zipCode << std::endl;
    return 0;
}

int main(int argc, char** argv)
{
    return cmdlime::CommandLineReader{"person-finder"}.exec<Cfg>(argc, argv, mainApp);
}
