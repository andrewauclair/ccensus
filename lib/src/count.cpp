#include "count.h"
#include "utils.h"

#include <fstream>

LineCounts counts_for_file(const std::string& path)
{
    LineCounts counts;

    auto file = std::ifstream(path);
    std::string line;

    bool in_block_comment = false;

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

    return counts;
}
