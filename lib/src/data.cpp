#include "data.h"
#include "utils.h"
#include <algorithm>
#include <fstream>
#include <set>
#include <iostream>

void Target::process()
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
                const auto path = entry.path().string();

                if (path.ends_with(".h") || path.ends_with(".hpp"))
                {
                    include_files.push_back(path);
                }
                if (path.ends_with(".c") || path.ends_with(".cc") || path.ends_with(".cxx") || path.ends_with(".cpp"))
                {
                    source_files.push_back(path);
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to read from directory " << path.path << "because: " << e.what() << '\n';
        }
    }

    std::set<std::string> files;
    files.insert(include_files.begin(), include_files.end());
    files.insert(source_files.begin(), source_files.end());

    total_counts = {};
    file_counts = {};
    bool in_block_comment = false;

    for (auto&& file_path : files)
    {
        auto file = std::ifstream(file_path);
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
        target.process();
    }

    std::sort(targets.begin(), targets.end(), 
        [](const Target& a, const Target& b) {
            return a.total_counts.total_lines < b.total_counts.total_lines;
        }
    );
}
