#include <cmdlime/config.h>
#include <iostream>
#include <sstream>

struct Coord{
    double lat;
    double lon;
};
std::stringstream& operator<<(std::stringstream& stream, const Coord& coord)
{
    stream << coord.lat << "-" << coord.lon;
    return stream;
}
std::stringstream& operator>>(std::stringstream& stream, Coord& coord)
{
    auto coordStr = std::string{};
    stream >> coordStr;
    auto delimPos = coordStr.find('-');
    if (delimPos == std::string::npos)
        throw cmdlime::Error{"Wrong coord format"};
    coord.lat = std::stod(coordStr.substr(0, delimPos));
    coord.lon = std::stod(coordStr.substr(delimPos + 1, coordStr.size() - delimPos - 1));
    return stream;
}


int main(int argc, char** argv)
{
    struct Cfg : public cmdlime::Config{
        ARG(zipCode, int)              << "zip code of the searched region";
        PARAM(surname, std::string)    << "surname of the person to find";
        PARAM(name, std::string)()     << "name of the person to find";
        PARAM(coord, Coord)            << "possible location";
        FLAG(verbose)                  << "adds more information to the output";
    } cfg;


    cfg.setVersionInfo("person-finder 1.0");
    auto reader = cmdlime::ConfigReader{cfg, "person-finder"};
    if (!reader.readCommandLine(argc, argv))
        return reader.exitCode();

    //At this point your config is ready to use
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode << std::endl;
    std::cout << "Possible location:" << cfg.coord.lat << " " << cfg.coord.lon;
    return 0;
}

