#ifndef CMAKE_H
#define CMAKE_H

#include "data.h"
#include "simdjson.h"
#include <string>

//struct parse_result {
//    simdjson::ondemand::parser parser;
//    simdjson::padded_string string;
//    simdjson::ondemand::document doc;
//};

class CMakeFrontend
{
public:
    CMakeFrontend(const std::string& build_directory);
    
    Package parse();

private:
    bool query();
    std::string read_index_file();
    std::string read_model_file(const std::string& index_file);

    //parse_result read_json(const std::string& path);

    // parses the targets from the reply json file
    Package parse_package(const std::string& model_file);

    // parse a single target from a reply target json file found from the main reply file
    Target parse_target(const std::string& source_directory, simdjson::ondemand::document& json);

    std::string m_build_directory;
};

#endif
