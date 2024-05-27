#ifndef CMAKE_H
#define CMAKE_H

#include "data.h"
#include "simdjson.h"
#include <string>

struct parse_result {
    simdjson::ondemand::parser parser;
    simdjson::padded_string string;
    simdjson::ondemand::document doc;
};

class CMakeFrontend
{
public:
    CMakeFrontend(const std::string& build_directory);
    
    Package parse();

private:
    void query();
    parse_result read_index_file();
    parse_result read_model_file(parse_result& json);

    parse_result read_json(const std::string& path);

    // parses the targets from the reply json file
    Package parse_package(parse_result& json);

    // parse a single target from a reply target json file found from the main reply file
    Target parse_target(const std::string& source_directory, parse_result& json);

    std::string m_build_directory;
};

#endif
