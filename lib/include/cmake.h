#ifndef CMAKE_H
#define CMAKE_H

#include "data.h"
#include "simdjson.h"
#include <string>

class CMakeFrontend
{
public:
    CMakeFrontend(const std::string& build_directory);
    
    Package parse();

private:
    void query();
    simdjson::ondemand::document read_index_file();
    simdjson::ondemand::document read_model_file(simdjson::ondemand::document& json);

    simdjson::ondemand::document read_json(const std::string& source_directory, const std::string& path);

    // parses the targets from the reply json file
    Package parse_package(simdjson::ondemand::object& json);

    // parse a single target from a reply target json file found from the main reply file
    Target parse_target(simdjson::ondemand::document& json);

    std::string m_build_directory;
};

#endif
