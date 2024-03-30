#ifndef LOC_SOLUTION_H
#define LOC_SOLUTION_H

#include "project.h"

#include <string>
#include <filesystem>
#include <map>

struct Solution
{
	std::string name;
	std::filesystem::path path;
	std::map<std::string, Project> projects;

	Solution(std::string name) : name(std::move(name))
	{
	}

	void process_files(bool verbose)
	{
		for (auto&& project : projects)
		{
			project.second.process_files(verbose);
		}
	}

	const std::map<std::string, Project>& projects_view() const
	{
		return projects;
	}

	std::int64_t total_projects() const { return projects.size(); }

	std::int64_t total_files() const
	{
		std::int64_t total = 0;
		for (auto&& project : projects) total += project.second.total_files();
		return total;
	}

	std::int64_t total_lines() const
	{
		std::int64_t total = 0;
		for (auto&& project : projects) total += project.second.counts.total_lines;
		return total;
	}

	std::int64_t physical_lines() const
	{
		std::int64_t total = 0;
		for (auto&& project : projects) total += project.second.counts.physical_lines();
		return total;
	}

	std::int64_t blank_lines() const
	{
		std::int64_t total = 0;
		for (auto&& project : projects) total += project.second.counts.blank_lines;
		return total;
	}

	std::int64_t comment_lines() const
	{
		std::int64_t total = 0;
		for (auto&& project : projects) total += project.second.counts.comment_lines;
		return total;
	}
};

Solution parse_solution(std::string_view name, std::istream& solution_file, const std::filesystem::path& solution_path, bool verbose);

//inline void process_solution(std::filesystem::path solution) {}
//
//inline void compare_solutions(std::filesystem::path solutionA, std::filesystem::path solutionB) {}

#endif