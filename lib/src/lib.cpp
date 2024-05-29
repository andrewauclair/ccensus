#include "lib.h"
#include "solution.h"
#include "display_utils.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <ranges>
#include <variant>
#include <array>
#include <format>

struct FileDiffs {
	std::optional<LineCounts> before;
	std::optional<LineCounts> after;
};

struct ProjectDiffs {
	std::map<std::string, FileDiffs> diffs;

	std::optional<LineCounts> before_totals() const
	{
		LineCounts counts;
		bool has_any = false;
		for (auto&& diffs : std::views::values(diffs))
		{
			if (diffs.before.has_value()) has_any = true;
			counts += diffs.before.value_or(LineCounts());
		}
		if (!has_any)
		{
			return std::nullopt;
		}
		return counts;
	}

	std::optional<LineCounts> after_totals() const
	{
		LineCounts counts;
		bool has_any = false;
		for (auto&& diffs : std::views::values(diffs))
		{
			if (diffs.after.has_value()) has_any = true;
			counts += diffs.after.value_or(LineCounts());
		}
		if (!has_any)
		{
			return std::nullopt;
		}
		return counts;
	}
};

void compare_solutions(const std::string& solution_a, const std::string& solution_b, OutputType outputType, OutputDetail outputDetail)
{
	std::ifstream solutionA_file(solution_a);
	std::ifstream solutionB_file(solution_b);

	std::filesystem::path solutionA_path = std::filesystem::path(solution_a).parent_path();
	std::filesystem::path solutionB_path = std::filesystem::path(solution_b).parent_path();

	auto solutionA = parse_solution(solutionA_path.filename().string(), solutionA_file, solutionA_path);
	auto solutionB = parse_solution(solutionB_path.filename().string(), solutionB_file, solutionB_path);

	solutionA.process_files();
	solutionB.process_files();

	//						A			B		Difference
	// Total Projects		2			3			+1
	// Total Files:
	// Total Lines:
	// Blank Lines:
	// Comment Lines:

	std::cout << "Before = " << solutionA_path << '\n';
	std::cout << "After  = " << solutionB_path << '\n';
	std::cout << "\n\n";

	Table<4> summary;
	summary.set_showpos[3] = true; // show difference with std::showpos
	summary.insert_row("", "Before", "After", "Difference");
	summary.insert_row("Projects", solutionA.total_projects(), solutionB.total_projects(), solutionB.total_projects() - solutionA.total_projects());
	summary.insert_row("Files", solutionA.total_files(), solutionB.total_files(), solutionB.total_files() - solutionA.total_files());
	summary.insert_row("Total Lines", solutionA.total_lines(), solutionB.total_lines(), solutionB.total_lines() - solutionA.total_lines());
	summary.insert_row("Physical Lines", solutionA.physical_lines(), solutionB.physical_lines(), solutionB.physical_lines() - solutionA.physical_lines());
	summary.insert_row("", "", "", "");
	summary.insert_row("", "", "", "");
	summary.insert_row("", "", "", "");

	std::map<std::string, ProjectDiffs> project_diffs;

	for (auto&& project : solutionA.projects_view())
	{
		auto& diffs = project_diffs[project.first];

		for (auto&& [file, counts] : project.second.files_view())
		{
			auto& file_diffs = diffs.diffs[file];

			file_diffs.before = counts;
		}
	}

	for (auto&& project : solutionB.projects_view())
	{
		auto& diffs = project_diffs[project.first];

		for (auto&& [file, counts] : project.second.files_view())
		{
			auto& file_diffs = diffs.diffs[file];

			file_diffs.after = counts;
		}
	}

	const auto build_row = [&summary, &outputDetail](const std::string& name, std::optional<LineCounts> before, std::optional<LineCounts> after)
		{
			std::variant<std::string, std::int64_t> before_lines = "";
			std::variant<std::string, std::int64_t> after_lines = "";
			std::int64_t total_diff = 0;

			if (before) before_lines = before.value().total_lines;
			if (after) after_lines = after.value().total_lines;
			if (before && after)
			{
				total_diff = after.value().total_lines - before.value().total_lines;
			}
			else if (before)
			{
				total_diff = -before.value().total_lines;
			}
			else if (after)
			{
				total_diff = after.value().total_lines;
			}
			std::variant<std::string, std::int64_t> diff = total_diff;
			if (total_diff == 0) diff = "";

			if (outputDetail != OutputDetail::ONLY_DIFFS || total_diff != 0)
			{
				summary.insert_row(name, before_lines, after_lines, diff);
			}
		};

	for (auto&& [project, diffs] : project_diffs)
	{
		build_row(project, diffs.before_totals(), diffs.after_totals());
		summary.insert_row("", "", "", "");

		for (auto&& [file, file_diffs] : diffs.diffs)
		{
			build_row(file, file_diffs.before, file_diffs.after);
		}
		summary.insert_row("", "", "", "");
		summary.insert_row("", "", "", "");
	}

	std::cout << summary;
	std::cout << "\n\n\n";
}

void single_solution(const std::string& solution_name, OutputType outputType, OutputDetail outputDetail)
{
	std::ifstream solution_file(solution_name);

	std::filesystem::path solution_path = std::filesystem::path(solution_name).parent_path();

	auto solution = parse_solution(solution_path.filename().string(), solution_file, solution_path);

	solution.process_files();

	std::cout << "Solution " << solution.name << "\n\n";
	
	Table<2> summary;
	summary.insert_row("Total Projects", solution.total_projects());
	summary.insert_row("Total Files", solution.total_files());
	summary.insert_row("Total Lines", solution.total_lines());

	std::cout << summary << "\n\n\n";

	for (auto&& project : std::views::values(solution.projects))
	{
		if (project.files.empty())
		{
			continue;
		}
		std::cout << "Project: " << project.name << "\n\n";

		Table<3> project_out;
		project_out.insert_row("File Name", "TLOC", "PLOC");

		for (auto&& [file_name, file_counts] : project.files)
		{
			project_out.insert_row(file_name, file_counts.total_lines, file_counts.physical_lines());
		}
		std::cout << project_out << "\n\n\n";
	}
}

void single_project(const std::string& project, OutputType outputType, OutputDetail outputDetail)
{

}