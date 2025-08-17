#include "cmake.h"
#include "utils.h"
#include <filesystem>
#include <fstream>



CMakeFrontend::CMakeFrontend(const std::string& build_directory)
    : m_build_directory(build_directory)
{
}

Package CMakeFrontend::parse()
{
    if (!query())
    {
        throw std::runtime_error("Failed to run cmake");
    }

    auto index_file = read_index_file();
    auto model_file = read_model_file(index_file);

    Package package = parse_package(model_file);

    const std::string query_directory = m_build_directory + "/.cmake/api/v1/query/cclient-ccensus/";

    // remove the query file now that we're done with it. this will let cmake cleanup the reply files next time it runs
	std::filesystem::remove(query_directory + "/codemodel-v2");

    return package;
}

// initialize the cmake file api query
bool CMakeFrontend::query()
{
    const std::string query_directory = m_build_directory + "/.cmake/api/v1/query/client-ccensus/";
    
    // write query file to build directory
    std::filesystem::create_directories(query_directory);

    {
        std::ofstream(query_directory + "/codemodel-v2");
    }

    // run cmake . in the build directory
    return std::system(std::string("(cd " + m_build_directory + " && cmake .)").c_str()) == 0;
}

std::string CMakeFrontend::read_index_file()
{
	const std::string reply_directory = m_build_directory + "/.cmake/api/v1/reply/";

    std::vector<std::filesystem::directory_entry> index_files;

    // read reply json files from build directory
    for (auto const& dir_entry : std::filesystem::directory_iterator(m_build_directory + "/.cmake/api/v1/reply/")) 
    {
        if (starts_with(dir_entry.path().filename().string(), "index-"))
        {
            index_files.push_back(dir_entry);
        }
    }

    if (index_files.empty())
    {
        throw std::runtime_error("No response from cmake. index-* files not found.");
    }

    std::sort(index_files.begin(), index_files.end(), [](const auto& a, const auto& b) {
        return a.path().filename() < a.path().filename();
    });

    const auto file = index_files.back().path().filename();

    return reply_directory + file.string();
}

std::string CMakeFrontend::read_model_file(const std::string& index_file)
{
    /*parse_result json;
    json.string = simdjson::padded_string::load(index_file);
    json.doc = json.parser.iterate(json.string);*/
    simdjson::ondemand::parser json;
    simdjson::padded_string json_string = simdjson::padded_string::load(index_file);
    simdjson::ondemand::document json_doc = json.iterate(json_string);

    const std::string reply_directory = m_build_directory + "/.cmake/api/v1/reply/";

    auto reply = json_doc["reply"];
    auto client = reply["client-ccensus"];
    auto codemodel = client["codemodel-v2"];
    auto file = std::string_view(codemodel["jsonFile"]);

    return reply_directory + std::string(file);
}
//
//parse_result CMakeFrontend::read_json(const std::string& path)
//{
//    parse_result result;
//    result.string = simdjson::padded_string::load(path);
//
//    result.doc = result.parser.iterate(result.string);
//    return result;
//}

Package CMakeFrontend::parse_package(const std::string& model_file)
{
    /*parse_result json;
    json.string = simdjson::padded_string::load(model_file);
    json.doc = json.parser.iterate(json.string);*/
    simdjson::ondemand::parser json;
    simdjson::padded_string json_string = simdjson::padded_string::load(model_file);
    simdjson::ondemand::document json_doc = json.iterate(json_string);
    const std::string reply_directory = m_build_directory + "/.cmake/api/v1/reply/";

    // TODO we should probably handle the case where there are multiple configs, but for now we'll just process the first config we find
    auto configurations = json_doc["configurations"];

    struct cmake_target
    {
        std::string id;
        std::string name;
        std::string jsonFile;
    };
    std::vector<cmake_target> target_cache;
    
    for (auto config : configurations)
    {
        auto projects = config["projects"];

        std::vector<std::string> project_names;
        for (auto project : projects)
        {
            project_names.push_back(std::string(std::string_view(project["name"])));
        }

        auto targets = config["targets"];

        for (auto target : targets)
        {
            const auto id = std::string(std::string_view(target["id"]));
            const auto name = std::string(std::string_view(target["name"]));
            std::string file = reply_directory + std::string(std::string_view(target["jsonFile"]));

            target_cache.push_back({ id, name, file });
        }

        // skip the rest of the configs for now
        break;
    }

    auto paths = json_doc["paths"];
    auto source_directory = std::string(std::string_view(paths["source"]));

    Package package;
    package.source_dir = std::filesystem::canonical(source_directory).string() + "/";
    std::replace(package.source_dir.begin(), package.source_dir.end(), '\\', '/');

    for (const cmake_target& cached_target : target_cache)
    {
        simdjson::ondemand::parser target_parser;
        simdjson::padded_string target_string = simdjson::padded_string::load(cached_target.jsonFile);
        simdjson::ondemand::document target_doc = target_parser.iterate(target_string);
        /*parse_result target_json;
        target_json.string = simdjson::padded_string::load(cached_target.jsonFile);
        target_json.doc = target_json.parser.iterate(target_json.string);*/

        Target target = parse_target(source_directory, target_doc);

        if (target.is_utility)
        {
            continue;
        }
        target.id = cached_target.id;

        package.targets.push_back(std::move(target));
    }
    
    for (auto& target : package.targets)
    {
        for (auto&& dependency : target.dependency_ids)
        {
            const auto result = std::find_if(package.targets.begin(), package.targets.end(), [&dependency](auto&& tar) { return tar.id == dependency; });

            if (result != package.targets.end())
            {
                target.target_dependencies.push_back(&*result);
            }
        }
    }

    std::size_t files = 0;

    for (auto&& target : package.targets)
    {
        files += target.files.size();
    }
    
    return package;
}

Target CMakeFrontend::parse_target(const std::string& source_directory, simdjson::ondemand::document& json)
{
    Target target;

    auto root = json.get_object();

    for (auto obj : root)
    {
	    auto basic_string_view = obj.unescaped_key().value();
        if (basic_string_view == "name")
        {
            target.name = std::string_view(obj.value());
        }

        if (basic_string_view == "type")
        {
            target.is_utility = std::string_view(obj.value()) == "UTILITY";
        }

        if (basic_string_view == "paths")
        {
            target.is_third_party = starts_with(std::string_view(obj.value()["build"]), "_deps");
        }

        if (basic_string_view == "dependencies")
        {
            for (auto dependency : obj.value())
            {
                target.dependency_ids.push_back(std::string(std::string_view(dependency["id"])));
            }
        }
        if (basic_string_view == "sources")
        {
            auto sources = obj.value();

            for (auto source : sources)
            {
                auto path = std::string_view(source["path"]);

                if (target.name == "ActiveRecord")
                {
                    std::cout << "file path from 'sources': " << path << '\n';
                }
                if (ends_with(path, ".h") || ends_with(path, ".hpp"))
                {
                    target.files.insert(std::string(path));
                }
                else if (ends_with(path, ".c") || ends_with(path, ".cc") || ends_with(path, ".cxx") || ends_with(path, ".cpp"))
                {
                    target.files.insert(std::string(path));
                }
            }
        }

        if (basic_string_view == "compileGroups")
        {
            for (auto group : obj.value().get_array())
            {
                for (auto item : group.value().get_object())
                {
                    if (item.key() == "includes")
                    {
                        for (auto path : item.value().get_array())
                        {
                            IncludePath includePath;

                            for (auto path_item : path.value().get_object())
                            {
                                if (path_item.key() == "backtrace")
                                {
                                    includePath.backtrace_index = path_item.value().get_uint64();
                                }
                                if (path_item.key() == "path")
                                {
                                    includePath.path = std::string_view(path_item.value());

                                    includePath.path = std::filesystem::absolute(includePath.path).generic_string();
                                }
                            }

                            target.include_paths.insert(includePath);
                        }
                    }
                }
            }
        }
        if (basic_string_view == "backtraceGraph")
        {
            for (auto trace : obj.value().get_object())
            {
                if (trace.key() == "commands")
                {
                    for (auto command : trace.value().get_array())
                    {
                        target.commands.emplace_back(std::string_view(command.value()));
                    }
                }
                if (trace.key() == "files")
                {
                    for (auto file : trace.value().get_array())
                    {
                        //target.files.insert(std::string(std::string_view(file.value())));

                        auto path = std::string_view(file.value());

                        if (target.name == "ActiveRecord")
                        {
                            std::cout << "file path from backtraceGraph files: " << path << '\n';
                        }

                        if (ends_with(path, ".h") || ends_with(path, ".hpp"))
                        {
                            target.files.insert(std::string(path));
                        }
                        else if (ends_with(path, ".c") || ends_with(path, ".cc") || ends_with(path, ".cxx") || ends_with(path, ".cpp"))
                        {
                            target.files.insert(std::string(path));
                        }
                    }
                }
                if (trace.key() == "nodes")
                {
                    for (auto node_obj : trace.value().get_array())
                    {
                        BacktraceNode backtrace_node;

                        for (auto node : node_obj.value().get_object())
                        {
                            if (node.key() == "command")
                            {
                                backtrace_node.command = node.value().get_uint64();
                            }
                            if (node.key() == "file")
                            {
                                backtrace_node.file = node.value().get_uint64();
                            }
                            if (node.key() == "line")
                            {
                                backtrace_node.line = node.value().get_uint64();
                            }
                            if (node.key() == "parent")
                            {
                                backtrace_node.parent = node.value().get_uint64();
                            }
                        }

                        target.backtrace_nodes.push_back(backtrace_node);
                    }
                }
            }
        }
    }
    
    return target;
}
