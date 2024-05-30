#include "cmake.h"
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

parse_result CMakeFrontend::read_index_file()
{
	const std::string reply_directory = m_build_directory + "/.cmake/api/v1/reply/";

    std::vector<std::filesystem::directory_entry> index_files;

    // read reply json files from build directory
    for (auto const& dir_entry : std::filesystem::directory_iterator(m_build_directory + "/.cmake/api/v1/reply/")) 
    {
        if (dir_entry.path().filename().string().starts_with("index-"))
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

    return read_json(reply_directory + file.string());
}

parse_result CMakeFrontend::read_model_file(parse_result& json)
{
    const std::string reply_directory = m_build_directory + "/.cmake/api/v1/reply/";

    auto reply = json.doc["reply"];
    auto client = reply["client-ccensus"];
    auto codemodel = client["codemodel-v2"];
    auto file = std::string_view(codemodel["jsonFile"]);

    return read_json(reply_directory + std::string(file));
}

parse_result CMakeFrontend::read_json(const std::string& path)
{
    parse_result result;
    result.string = simdjson::padded_string::load(path);

    result.doc = result.parser.iterate(result.string);
    return result;
}

Package CMakeFrontend::parse_package(parse_result& json)
{
    const std::string reply_directory = m_build_directory + "/.cmake/api/v1/reply/";

    // TODO we should probably handle the case where there are multiple configs, but for now we'll just process the first config we find
    auto configurations = json.doc["configurations"];

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
            const auto id = std::string_view(target["id"]);
            const auto name = std::string_view(target["name"]);

            target_cache.emplace_back(std::string(id), std::string(name), reply_directory + std::string(std::string_view(target["jsonFile"])));
        }
    }

    auto paths = json.doc["paths"];
    auto source_directory = std::string(std::string_view(paths["source"]));

    Package package;

    for (const cmake_target& cached_target : target_cache)
    {
        auto target_json = read_json(cached_target.jsonFile);

        Target target = parse_target(source_directory, target_json);

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
        files += target.include_files.size();
        files += target.source_files.size();
    }
    
    return package;
}

Target CMakeFrontend::parse_target(const std::string& source_directory, parse_result& json)
{
    Target target;

    auto root = json.doc.get_object();

    for (auto obj : root)
    {
        if (obj.key() == "name")
        {
            target.name = std::string_view(obj.value());
        }

        if (obj.key() == "type")
        {
            target.is_utility = std::string_view(obj.value()) == "UTILITY";
        }

        if (obj.key() == "paths")
        {
            target.is_third_party = std::string_view(obj.value()["build"]).starts_with("_deps");
        }

        if (obj.key() == "dependencies")
        {
            for (auto dependency : obj.value())
            {
                target.dependency_ids.push_back(std::string(std::string_view(dependency["id"])));
            }
        }
        if (obj.key() == "sources")
        {
            auto sources = obj.value();

            for (auto source : sources)
            {
                auto path = std::string_view(source["path"]);

                if (path.ends_with(".h") || path.ends_with(".hpp"))
                {
                    target.include_files.push_back(source_directory + "/" + std::string(path));
                }
                else if (path.ends_with(".c") || path.ends_with(".cc") || path.ends_with(".cxx") || path.ends_with(".cpp"))
                {
                    target.source_files.push_back(source_directory + "/" + std::string(path));
                }
            }
        }

        if (obj.key() == "compileGroups")
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
                                }
                            }

                            target.include_paths.push_back(includePath);
                        }
                    }
                }
            }
        }
        if (obj.key() == "backtraceGraph")
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
                        target.files.emplace_back(std::string_view(file.value()));
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
