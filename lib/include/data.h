#ifndef CCENSUS_DATA_H
#define CCENSUS_DATA_H

#include <cstdint>
#include <vector>
#include <filesystem>
#include <string>
#include <map>
#include <optional>

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

struct BacktraceNode
{
	std::optional<std::uint32_t> command;
	std::optional<std::uint32_t> file;
	std::optional<std::uint32_t> line;
	std::optional<std::uint32_t> parent;
};

struct IncludePath
{
	std::uint32_t backtrace_index;
	std::string path;
};

struct Target
{
	std::string id;
    std::string name;
    std::string path;

	std::vector<IncludePath> include_paths;

	// not sure we can actually tell if something is 3rd party in VS solutions
	// but we should be able to do this for cmake
    bool is_third_party = false;

	// checks if this target is a cmake project or an executable in VS
	bool is_project = false;
	
	// flag to check if this is a cmake utility. we'll skip those targets
	// TODO skipping utility targets might be an issue if they generate C++ code?
	bool is_utility = false;

	LineCounts total_counts;
    std::map<std::string, LineCounts> file_counts;

	std::vector<Target*> target_dependencies;
	std::vector<std::string> dependency_ids;

	std::vector<std::string> commands;
	std::vector<std::string> files;

	std::vector<BacktraceNode> backtrace_nodes;

	bool is_node_target_include_directory(std::uint32_t node) const
	{
		if (backtrace_nodes.at(node).command)
		{
			const auto index = backtrace_nodes.at(node).command.value();
			
			return commands.at(index) == "target_include_directories";
		}
		return false;
	}
	void process();
};

struct Package
{
    std::vector<Target> targets;

	void process();
};

#endif
