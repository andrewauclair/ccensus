#include "lib.h"
#include "solution.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <ranges>







std::int64_t width_for_value(std::int64_t value)
{
	std::int64_t width = 1;
	std::int64_t temp = 1;

	while (temp < value)
	{
		temp *= 10;
		width++;
	}
	return width;
}

std::int64_t width_for_values(auto value, auto... values)
{
	return std::max({ width_for_value(value), width_for_value(values)... });
}

std::int64_t width_for_values(auto view)
{
	std::int64_t max = 0;

	for (auto&& v : view)
	{
		max = std::max(max, width_for_value(v));
	}
	return max;
}

std::int64_t longest_string(auto view)
{
	std::int64_t max = 0;

	for (auto&& v : view)
	{
		max = std::max(max, static_cast<std::int64_t>(v.length()));
	}
	return max;
}

void compare_solutions(const std::string& solution_a, const std::string& solution_b, OutputType outputType, OutputDetail outputDetail)
{
	/*bool display_details = false;

	if (argc > 4 && std::string_view(argv[4]) == "--display-details")
	{
		display_details = true;
	}*/
	bool verbose = false;

	std::ifstream solutionA_file(solution_a);
	std::ifstream solutionB_file(solution_b);

	std::filesystem::path solutionA_path = std::filesystem::path(solution_a).parent_path();
	std::filesystem::path solutionB_path = std::filesystem::path(solution_b).parent_path();

	auto solutionA = parse_solution(solutionA_path.filename().string(), solutionA_file, solutionA_path, verbose);
	auto solutionB = parse_solution(solutionB_path.filename().string(), solutionB_file, solutionB_path, verbose);

	solutionA.process_files(verbose);
	solutionB.process_files(verbose);

	//						A			B		Difference
	// Total Projects		2			3			+1
	// Total Files:
	// Total Lines:
	// Blank Lines:
	// Comment Lines:

	const int first_column_spacing = 16;
	const int spacing = std::max((std::int64_t)14, width_for_values(solutionA.total_projects(), solutionA.total_files(), solutionA.total_lines()));

	std::cout << "Before = " << solutionA_path << '\n';
	std::cout << "After  = " << solutionB_path << '\n';
	std::cout << "\n\n";
	std::cout << std::left << std::setw(first_column_spacing) << std::setfill(' ') << ' ';
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << "Before";// solutionA.name;
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << "After";// solutionB.name;
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << "Difference";
	std::cout << '\n';

	std::cout << std::left << std::setw(first_column_spacing) << std::setfill(' ') << "Total Projects: ";
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionA.total_projects();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionB.total_projects();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << std::showpos << (solutionB.total_projects() - solutionA.total_projects()) << std::noshowpos;
	std::cout << '\n';

	std::cout << std::left << std::setw(first_column_spacing) << std::setfill(' ') << "Total Files: ";
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionA.total_files();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionB.total_files();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << std::showpos << (solutionB.total_files() - solutionA.total_files()) << std::noshowpos;
	std::cout << '\n';

	std::cout << std::left << std::setw(first_column_spacing) << std::setfill(' ') << "Total Lines: ";
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionA.total_lines();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionB.total_lines();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << std::showpos << (solutionB.total_lines() - solutionA.total_lines()) << std::noshowpos;
	std::cout << '\n';

	std::cout << std::left << std::setw(first_column_spacing) << std::setfill(' ') << "Blank Lines: ";
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionA.blank_lines();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionB.blank_lines();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << std::showpos << (solutionB.blank_lines() - solutionA.blank_lines()) << std::noshowpos;
	std::cout << '\n';

	std::cout << std::left << std::setw(first_column_spacing) << std::setfill(' ') << "Comment Lines: ";
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionA.comment_lines();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << solutionB.comment_lines();
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << std::showpos << (solutionB.comment_lines() - solutionA.comment_lines()) << std::noshowpos;
	std::cout << '\n';

	std::size_t longest = 0;

	for (auto&& project : solutionA.projects_view())
	{
		longest = std::max(longest, project.second.name.length());

		for (auto&& file : project.second.files_view())
		{
			longest = std::max(longest, file.first.length());
		}
	}

	std::cout << "\n\n";
	std::cout << std::left << std::setw(longest) << std::setfill(' ') << "Modified";
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << "Before";// solutionA.name;
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << "After";// solutionB.name;
	std::cout << std::right << std::setw(spacing) << std::setfill(' ') << "Difference";
	std::cout << '\n';

	std::vector<std::string> deleted;
	std::vector<std::string> added;

	for (auto&& project : solutionA.projects_view())
	{
		auto projectB = solutionB.projects_view().find(project.first);

		std::cout << '\n';
		std::cout << std::left << std::setw(longest) << project.second.name;

		std::cout << std::right << std::setw(spacing) << std::setfill(' ') << project.second.counts.total_lines;

		if (projectB != solutionB.projects_view().end())
		{
			std::cout << std::right << std::setw(spacing) << std::setfill(' ') << projectB->second.counts.total_lines;
		}

		std::cout << '\n';

		for (auto&& file : project.second.files_view())
		{

			std::cout << std::left << std::setw(longest) << file.first;
			std::cout << std::right << std::setw(spacing) << std::setfill(' ') << file.second.total_lines;

			if (projectB != solutionB.projects_view().end())
			{
				auto fileB = projectB->second.files.find(file.first);

				if (fileB != projectB->second.files.end())
				{
					std::cout << std::right << std::setw(spacing) << std::setfill(' ') << fileB->second.total_lines;
				}
			}

			std::cout << '\n';
		}
	}
}

void single_solution(const std::string& solution_name, OutputType outputType, OutputDetail outputDetail)
{
	bool verbose = false;

	std::ifstream solution_file(solution_name);

	std::filesystem::path solution_path = std::filesystem::path(solution_name).parent_path();

	auto solution = parse_solution(solution_path.filename().string(), solution_file, solution_path, verbose);

	solution.process_files(verbose);

	std::cout << "Solution " << solution.name << '\n';
	
	auto width = width_for_values(solution.total_projects(), solution.total_files(), solution.total_lines(), solution.blank_lines(), solution.comment_lines());

	std::cout << "Total Projects: " << std::setw(width) << std::right << solution.total_projects() << '\n';
	std::cout << "Total Files:    " << std::setw(width) << std::right << solution.total_files() << '\n';
	std::cout << "Total Lines:    " << std::setw(width) << std::right << solution.total_lines() << '\n';
	std::cout << "Blank Lines:    " << std::setw(width) << std::right << solution.blank_lines() << '\n';
	std::cout << "Comment Lines:  " << std::setw(width) << std::right << solution.comment_lines() << '\n';
	std::cout << "\n\n";

	for (auto&& project : std::views::values(solution.projects))
	{
		if (project.files.empty())
		{
			continue;
		}
		std::cout << "Project: " << project.name << "\n\n";

		auto longest_file_name = longest_string(std::views::keys(project.files));
		auto longest_total_lines = width_for_values(std::views::values(project.files) | std::views::transform([](const LineCounts& lines) { return lines.total_lines; }));
		auto longest_physical_lines = width_for_values(std::views::values(project.files) | std::views::transform([](const LineCounts& lines) { return lines.physical_lines(); }));

		longest_file_name = std::max(longest_file_name, (std::int64_t)9);
		longest_total_lines = std::max(longest_total_lines, (std::int64_t)5);
		longest_physical_lines = std::max(longest_physical_lines, (std::int64_t)5);

		std::cout << std::left << std::setw(longest_file_name) << "File Name" 
			<< std::right << std::setw(longest_total_lines) << "   TLOC " 
			<< std::right << std::setw(longest_physical_lines) << "  PLOC\n";

		for (auto&& [file_name, file_counts] : project.files)
		{
			std::cout << std::left << std::setw(longest_file_name) << file_name << "  "
				<< std::right << std::setw(longest_total_lines) << file_counts.total_lines << "  "
				<< std::right << std::setw(longest_physical_lines) << file_counts.physical_lines()
				<< '\n';
		}
		std::cout << "\n\n";
	}
}

void single_project(const std::string& project, OutputType outputType, OutputDetail outputDetail)
{

}