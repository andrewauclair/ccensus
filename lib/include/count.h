#ifndef CCENSUS_COUNT_H
#define CCENSUS_COUNT_H

#include <cstdint>
#include <string>

struct LineCounts
{
	std::int64_t total_lines = 0;
	std::int64_t code_only_lines = 0;
	std::int64_t code_and_comment_lines = 0;
	std::int64_t comment_lines = 0;
	std::int64_t blank_lines = 0;

	std::int64_t physical_lines() const
	{
		auto count = total_lines;

		count -= blank_lines;
		count -= comment_lines - code_and_comment_lines;

		return count;
	}

	void operator+=(const LineCounts& counts)
	{
		total_lines += counts.total_lines;
		code_only_lines += counts.code_only_lines;
		code_and_comment_lines += counts.code_and_comment_lines;
		comment_lines += counts.comment_lines;
		blank_lines += counts.blank_lines;
	}

    friend LineCounts operator-(const LineCounts& lhs, const LineCounts& rhs)
    {
        LineCounts counts;

        counts.total_lines = lhs.total_lines - rhs.total_lines;
        counts.code_only_lines = lhs.code_only_lines - rhs.code_only_lines;
        counts.code_and_comment_lines = lhs.code_and_comment_lines - rhs.code_and_comment_lines;
        counts.comment_lines = lhs.comment_lines - rhs.comment_lines;
        counts.blank_lines = lhs.blank_lines - rhs.blank_lines;
        
        return counts;
    }
};

LineCounts counts_for_file(const std::string& path);

#endif
