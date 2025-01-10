#ifndef CCENSUS_DATA_H
#define CCENSUS_DATA_H

#include "count.h"

#include <cstdint>
#include <vector>
#include <filesystem>
#include <string>
#include <map>
#include <optional>
#include <set>

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
private:
	LineCounts m_total_counts;
public:
    std::map<std::string, LineCounts> file_counts;

	std::vector<Target*> target_dependencies;
	std::vector<std::string> dependency_ids;

	std::vector<std::string> commands;
	std::set<std::string> files;

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
	void process(const std::string& source_directory);

	void calculate_total_counts();

	const LineCounts& total_counts() const { return m_total_counts; }
};

struct Package
{
	std::string source_dir;
	std::string output_file = "test.json";

    std::vector<Target> targets;

	void process();
};

#endif
