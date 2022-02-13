///examples/ex15.cpp
///
#include <cmdlime/config.h>
#include <algorithm>

struct EnsureAlpha{
    void operator()(const std::string& name)
    {
        if (!std::all_of(std::begin(name), std::end(name),
                         [](auto ch){
                             return std::isalpha(static_cast<int>(ch));
                         }))
            throw cmdlime::ValidationError{"value must contain alphabet characters only."};
    }
};

int main(int argc, char** argv)
{
    struct RecordCfg: public cmdlime::Config{
        CMDLIME_PARAM(file, std::string)() << "save result to file";
        CMDLIME_PARAM(db, std::string)()   << "save result to database";
        CMDLIME_FLAG(detailed)             << "hide search results" << cmdlime::WithoutShortName{};
    };

    struct HistoryCfg: public cmdlime::Config{
        CMDLIME_PARAM(surname, std::string)() << "filter search queries by surname" << EnsureAlpha{};
        CMDLIME_FLAG(noResults)               << "hide search results";
    };

    struct Cfg : public cmdlime::Config{
        CMDLIME_ARG(zipCode, int)             << "zip code of the searched region";
        CMDLIME_PARAM(surname, std::string)   << "surname of the person to find" << EnsureAlpha{};
        CMDLIME_PARAM(name, std::string)()    << "name of the person to find" 	 << EnsureAlpha{};
        CMDLIME_FLAG(verbose)                 << "adds more information to the output";
        CMDLIME_SUBCOMMAND(record, RecordCfg) << "record search result"
                                              << [](auto& record){
                                                  if (record && record->file.empty() && record->db.empty())
                                                      throw cmdlime::ValidationError{"file or db paremeter must be provided."};
                                                  else
                                                      throw std::runtime_error{"ERROR"};
                                              };
        CMDLIME_COMMAND(history, HistoryCfg)  << "show search history";
    } cfg;

    auto reader = cmdlime::ConfigReader{cfg, "person-finder"};
    if (!reader.readCommandLine(argc, argv))
        return reader.exitCode();

    if (cfg.history.has_value()){
        std::cout << "Preparing search history with surname filter:" << cfg.history->surname << std::endl;
        return 0;
    }

    std::cout << "Looking for person " << cfg.name << " " << cfg.surname << " in the region with zip code: " << cfg.zipCode << std::endl;
    if (cfg.record.has_value())
        std::cout << "Record settings: " << "file:" << cfg.record->file << " db:" << cfg.record->db << " detailed:" << cfg.record->detailed << std::endl;

    return 0;
}
