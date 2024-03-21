#define _CRT_SECURE_NO_WARNINGS

#include "project.h"
#include "solution.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <span>
#include <algorithm>
#include <map>



/*
* add arguments. I would like a way for this to run in an entire folder, finding any solutions in it and parsing those.
* --verbose to print extra into to the console
* single solution option
* single project option
* options to list out solutions or projects to specifically parse
* 
* output options: just dumping to console or printing a csv
* 
* 
*/
// TODO project total lines should count projects that it depends on. The csv output will have a bunch of different pieces of info.
// --compare-solutions <solution-a> <solution-b>
// --compare-projects <project-a> <project-b>
// --single-solution <solution>
// --single-project <project>
//
// by default output is in a table in the console, to format as csv for dumping to a file and opening in excel, append --csv 

//				Total Lines		Total Lines w/ dependencies		   Code Lines
// Solution A             X							      					X
// Project Aa             X							      X					X
int main(int argc, char** argv)
{
	if (argc < 3)
	{
		return -1;
	}

	auto clock_start = std::chrono::system_clock::now();

	bool verbose = true;

	if (std::string(argv[1]) == "--compare-solutions")
	{
		if (argc < 4)
		{
			return -1;
		}

		bool display_details = false;

		if (argc > 4 && std::string_view(argv[4]) == "--display-details")
		{
			display_details = true;
		}

		std::ifstream solutionA_file(argv[2]);
		std::ifstream solutionB_file(argv[3]);

		std::filesystem::path solutionA_path = std::filesystem::path(argv[2]).parent_path();
		std::filesystem::path solutionB_path = std::filesystem::path(argv[3]).parent_path();

		auto solutionA = parse_solution(solutionA_path.filename().string(), solutionA_file, solutionA_path, verbose);
		auto solutionB = parse_solution(solutionB_path.filename().string(), solutionB_file, solutionB_path, verbose);

		solutionA.process_files();
		solutionB.process_files();

		//						A			B		Difference
		// Total Projects		2			3			+1
		// Total Files:
		// Total Lines:
		// Blank Lines:
		// Comment Lines:

		const int first_column_spacing = 16;
		const int spacing = 14;

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
	else if (std::string(argv[1]) == "--compare-projects")
	{
		if (argc < 4)
		{
			return -1;
		}

	}
	else if (std::string(argv[1]) == "--single-solution")
	{
		std::ifstream solution_file(argv[2]);

		std::filesystem::path solution_path = std::filesystem::path(argv[2]).parent_path();

		auto solution = parse_solution(solution_path.filename().string(), solution_file, solution_path, verbose);

		solution.process_files();

		std::cout << "Solution " << solution.name << '\n';
		std::cout << "Total Projects: " << solution.total_projects() << '\n';
		std::cout << "Total Files:    " << solution.total_files() << '\n';
		std::cout << "Total Lines:    " << solution.total_lines() << '\n';
		std::cout << "Blank Lines:    " << solution.blank_lines() << '\n';
		std::cout << "Comment Lines:  " << solution.comment_lines() << '\n';
	}
	else if (std::string(argv[1]) == "--single-project")
	{
		//parse_project()
	}

	auto clock_now = std::chrono::system_clock::now();
	auto elapsed_time = std::chrono::duration_cast <std::chrono::milliseconds> (clock_now - clock_start).count();
	std::cout << "\nElapsed Time:   " << elapsed_time << "ms \n";
}
