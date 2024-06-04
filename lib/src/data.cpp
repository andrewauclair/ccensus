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

                if (path.starts_with(source_directory))
                {
                    path.erase(0, source_directory.size());
                }

                if (path.ends_with(".h") || path.ends_with(".hpp"))
                {
                    files.push_back(path);
                }
                if (path.ends_with(".c") || path.ends_with(".cc") || path.ends_with(".cxx") || path.ends_with(".cpp"))
                {
                    files.push_back(path);
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to read from directory " << path.path << "\n    because: " << e.what() << '\n';
        }
    }

    total_counts = {};
    file_counts = {};

    for (auto&& file_path : files)
    {
        auto path = file_path;

        if (!std::filesystem::path(path).is_absolute())
        {
            path = source_directory + file_path;
        }

        file_counts[file_path] = counts_for_file(path);
        total_counts += file_counts[file_path];
	}
}

void Package::process()
{
    for (Target& target : targets)
    {
        target.process(source_dir);
    }

    std::sort(targets.begin(), targets.end(), 
        [](const Target& a, const Target& b) {
            return a.total_counts.total_lines < b.total_counts.total_lines;
        }
    );
}
