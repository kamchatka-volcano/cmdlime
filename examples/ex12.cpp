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

