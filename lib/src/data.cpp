#include "data.h"
#include "utils.h"
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
    bool in_block_comment = false;

    for (auto&& file_path : files)
    {
        auto path = file_path;

        if (!std::filesystem::path(path).is_absolute())
        {
            path = source_directory + file_path;
        }
        auto file = std::ifstream(path);
        std::string line;

        LineCounts counts;

        while (std::getline(file, line))
        {
            counts.total_lines++;

            if (!in_block_comment)
            {
                if (is_blank(line))
                {
                    counts.blank_lines++;
                }

                if (is_single_comment(line) || is_opening_block_comment(line) == Block_Comment_Type::Inline_Comment)
                {
                    counts.comment_lines++;
                }

                if (is_code_and_comment(line))
                {
                    counts.code_and_comment_lines++;
                }
            }

            if (!in_block_comment && is_opening_block_comment(line) == Block_Comment_Type::Opening)
            {
                // TODO need to check if there was any code on this line too
                in_block_comment = true;
                counts.comment_lines++;
            }
            else if (in_block_comment && is_closing_block_comment(line))
            {
                in_block_comment = false;
                counts.comment_lines++;
            }
            else if (in_block_comment)
            {
                counts.comment_lines++;
            }
        }

        file_counts[file_path] = counts;
        total_counts += counts;
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
