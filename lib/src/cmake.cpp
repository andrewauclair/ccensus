#include "cmake.h"
#include <filesystem>
#include <fstream>

CMakeFrontend::CMakeFrontend(const std::string& build_directory)
    : m_build_directory(build_directory)
{
}

void CMakeFrontend::parse()
{
    query();

    auto index_file = read_index_file();
    auto model_file = read_model_file(index_file);

    return parse_package(model_file);
}

// initialize the cmake file api query
void CMakeFrontend::query()
{
    const std::string query_directory = m_build_directory + "/.cmake/api/v1/query/cclient-ccensus/";
    // write query file to build directory
    std::filesystem::create_directories(query_directory);

    {
        std::ofstream(query_directory + "/codemodel-v2");
    }

    // run cmake . in the build directory
    std::system(std::string("(cd " + m_build_directory + " && cmake .)").c_str());

    // remove the query file now that we're done with it. this will let cmake cleanup the reply files next time it runs
	std::filesystem::remove(query_directory + "/codemodel-v2");
}

simdjson::ondemand::document CMakeFrontend::read_index_file()
{
	const std::string reply_directory = m_build_directory + "/.cmake/api/v1/reply/";

    std::vector<std::filesystem::directory_entry> index_files;

    // read reply json files from build directory
    for (auto const& dir_entry : std::filesystem::directory_iterator(build_directory + "/.cmake/api/v1/reply/")) 
    {
        if (dir_entry.path().filename().string().starts_with("index-"))
        {
            index_files.push_back(dir_entry);
        }
    }

    // TODO if there are no index files then something is wrong

    std::sort(index_files.begin(), index_files.end(), [](const auto& a, const auto& b) {
        return a.path().filename() < a.path().filename();
    });

    const auto file = index_files.back().path().filename();

    return read_json(file);
}

simdjson::ondemand::document CMakeFrontend::read_model_file(simdjson::ondemand::document& json)
{
    const std::string reply_directory = m_build_directory + "/.cmake/api/v1/reply/";

    auto file = json["reply"]["client-ccensus"]["codemodel-v2"]["jsonFile"];

    return read_json(file);
}

simdjson::ondemand::document CMakeFrontend::read_json(const std::string& path)
{
    simdjson::ondemand::parser parser;
    simdjson::padded_string json = simdjson::padded_string::load(path);

    return parser.iterate(json);
}

Package CMakeFrontend::parse_package(simdjson::ondemand::object& json)
{
    const std::string reply_directory = m_build_directory + "/.cmake/api/v1/reply/";

    // TODO we should probably handle the case where there are multiple configs, but for now we'll just process the first config we find
    auto configurations = json["configurations"];

    for (auto config : configurations)
    {
        auto targets = config["targets"];

        struct cmake_target
        {
            std::string name;
            std::string jsonFile;
        };
        std::vector<cmake_target> target_cache;

        for (auto target : targets)
        {
            const auto name = target["name"];

            target_cache.emplace_back(name, reply_directory + target["jsonFile"]);
        }

        auto paths = config["paths"];
        auto source_directory = paths["source"];

        // return package
    }

    // TODO throw exception that we didn't parse a package
}

Target CMakeFrontend::parse_target(const std::string& source_directory, simdjson::ondemand::document& json)
{
    Target target;

    auto sources = json["sources"];

    for (auto source : sources)
    {
        auto path = source["path"];

        target.source_files.push_back(source_directory + "/" + path);
    }

    json["type"];
}
