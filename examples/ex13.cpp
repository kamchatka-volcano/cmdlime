///examples/ex13.cpp
///
#include <cmdlime/commandlinereader.h>

struct RecordCfg: public cmdlime::Config{
    CMDLIME_PARAM(file, std::string)() << "save result to file";
    CMDLIME_PARAM(db, std::string)()   << "save result to database";
    CMDLIME_FLAG(detailed)             << "adds more information to the result" << cmdlime::WithoutShortName{};
};

struct Cfg : public cmdlime::Config{
    CMDLIME_ARG(zipCode, int)               << "zip code of the searched region";
    CMDLIME_PARAM(surname, std::string)     << "surname of the person to find";
    CMDLIME_PARAM(name, std::string)()      << "name of the person to find";
    CMDLIME_FLAG(verbose)                   << "adds more information to the output";
    CMDLIME_SUBCOMMAND(record, RecordCfg)   << "record search result";
};

int mainApp(const Cfg& cfg)
{
    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode << std::endl;
    if (cfg.record.has_value())
        std::cout << "Record settings: " << "file:" << cfg.record->file << " db:" << cfg.record->db << " detailed:" << cfg.record->detailed << std::endl;
    return 0;
}

int main(int argc, char** argv)
{
    return cmdlime::CommandLineReader{"person-finder"}.exec<Cfg>(argc, argv, mainApp);
}
