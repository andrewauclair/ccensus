#include "data.h"
#include "utils.h"
#include <algorithm>
#include <fstream>

void Target::process()
{
    std::vector<std::string> files;
    files.insert(files.end(), include_files.begin(), include_files.end());
    files.insert(files.end(), source_files.begin(), source_files.end());

    for (auto&& path : include_paths)
    {
        if (!is_node_target_include_directory(path.backtrace_index))
        {
            continue;
        }
        for (const auto& entry : std::filesystem::directory_iterator(path.path))
        {
            files.push_back(entry.path().string());
        }
    }

    std::sort(files.begin(), files.end());

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
}
