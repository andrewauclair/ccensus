#include "data.h"
#include "utils.h"
#include "count.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <iostream>

void Target::process(const std::string& source_directory)
{
    for (auto&& path : include_paths)
    {
        if (!is_node_target_include_directory(path.backtrace_index))
        {
            continue;
        }
        
        try
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path.path))
            {
                if (entry.is_directory())
                {
                    continue;
                }
                auto path = entry.path().string();
                std::replace(path.begin(), path.end(), '\\', '/');

                if (starts_with(path, source_directory))
                {
                    path.erase(0, source_directory.size());
                }

                if (name == "ActiveRecord")
                {
                    std::cout << "file path from include_paths: " << path << '\n';
                }

                if (ends_with(path, ".h") || ends_with(path, ".hpp"))
                {
                    files.insert(path);
                }
                if (ends_with(path, ".c") || ends_with(path, ".cc") || ends_with(path, ".cxx") || ends_with(path, ".cpp"))
                {
                    files.insert(path);
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to read from directory " << path.path << "\n    because: " << e.what() << '\n';
        }
    }

    file_counts = {};

    for (auto&& file_path : files)
    {
        std::string path = file_path;

        if (!std::filesystem::path(path).is_absolute())
        {
            path = source_directory + file_path;
        }

        file_counts[file_path] = counts_for_file(path);
        // total_counts += file_counts[file_path];
	}

    calculate_total_counts();
}

void Package::process()
{
    for (Target& target : targets)
    {
        target.process(source_dir);
    }

    std::sort(targets.begin(), targets.end(), 
        [](const Target& a, const Target& b) {
            return a.total_counts().total_lines < b.total_counts().total_lines;
        }
    );
}

void Target::calculate_total_counts()
{
    m_total_counts = {};

    for (auto&& file : file_counts)
    {
        m_total_counts += file.second;
    }
}