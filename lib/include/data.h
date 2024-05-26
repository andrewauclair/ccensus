#ifndef CCENSUS_DATA_H
#define CCENSUS_DATA_H

#include <cstdint>
#include <vector>
#include <filesystem>
#include <string>
#include <map>

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
};

struct Target
{
    std::string name;
    std::string path;
    std::vector<std::string> include_files;
    std::vector<std::string> source_files;

	// not sure we can actually tell if something is 3rd party in VS solutions
	// but we should be able to do this for cmake
    bool is_third_party = false;

	// checks if this target is a cmake project or an executable in VS
	bool is_project = false;
	
    std::map<std::string, LineCounts> counts;
};

struct Package
{
    std::vector<Target> targets;
};

#endif
