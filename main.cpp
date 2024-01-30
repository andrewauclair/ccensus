#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <span>
#include <algorithm>
#include <map>

bool is_blank(std::string_view string)
{
	for (auto ch : string)
	{
		if (ch != ' ' && ch != '\t') return false;
	}
	return true;
}

bool is_single_comment(std::string_view string)
{
	char prev_ch = ' ';

	for (auto ch : string)
	{
		if (ch == '/' && prev_ch == '/') return true;

		prev_ch = ch;
	}
	return false;
}

enum class Block_Comment_Type
{
	None, // there is no block comment
	Inline_Comment, // there is a block comment, but it's contained within this line
	Opening // there is a opening block comment and the closing block comment is on a future line
};

Block_Comment_Type is_opening_block_comment(std::string_view string)
{
	bool in_quotes = false;
	char prev_ch = ' ';
	Block_Comment_Type type = Block_Comment_Type::None;

	for (auto ch : string)
	{
		if (!in_quotes && (ch == '\'' || ch == '"')) in_quotes = true;
		else if (in_quotes && (ch == '\'' || ch == '"') && prev_ch != '\\') in_quotes = false;
		else if (!in_quotes && prev_ch == '/' && ch == '*') type = Block_Comment_Type::Opening;
		else if (!in_quotes && prev_ch == '*' && ch == '/') type = Block_Comment_Type::Inline_Comment;

		prev_ch = ch;
	}

	return type;
}

// this function will find a closing block comment of the form */ in the string
// only called when we know we're inside of a block comment
bool is_closing_block_comment(std::string_view string)
{
	char prev_ch = ' ';

	for (auto ch : string)
	{
		if (prev_ch == '*' && ch == '/') return true;

		prev_ch = ch;
	}

	return false;
}

struct LineCounts
{
	std::int64_t total_lines = 0;
	std::int64_t code_only_lines = 0;
	std::int64_t code_and_comment_lines = 0;
	std::int64_t comment_lines = 0;
	std::int64_t blank_lines = 0;

	void operator+=(const LineCounts& counts)
	{
		total_lines += counts.total_lines;
		code_only_lines += counts.code_only_lines;
		code_and_comment_lines += counts.code_and_comment_lines;
		comment_lines += counts.comment_lines;
		blank_lines += counts.blank_lines;
	}
};

struct Project 
{
	std::string name;
	std::filesystem::path path;
	std::vector<std::filesystem::path> file_paths;
	std::map<std::string, LineCounts> files;

	LineCounts counts;

	friend bool operator<(const Project& lhs, const Project& rhs)
	{
		return lhs.name < rhs.name;
	}

	Project() = default;

	Project(std::string name) : name(std::move(name))
	{
	}

	const std::map<std::string, LineCounts>& files_view() const
	{
		return files;
	}

	void process_files()
	{
		std::sort(file_paths.begin(), file_paths.end());

		counts = {};
		bool in_block_comment = false;

		for (auto&& file_path : file_paths)
		{
			auto file = std::ifstream(file_path);
			std::string line;

			LineCounts file_counts;

			while (std::getline(file, line))
			{
				file_counts.total_lines++;

				if (!in_block_comment)
				{
					if (is_blank(line))
					{
						file_counts.blank_lines++;
					}

					if (is_single_comment(line))
					{
						file_counts.comment_lines++;
					}
				}

				if (!in_block_comment && is_opening_block_comment(line) == Block_Comment_Type::Opening)
				{
					// TODO need to check if there was any code on this line too
					in_block_comment = true;
					file_counts.comment_lines++;
				}
				else if (in_block_comment && is_closing_block_comment(line))
				{
					in_block_comment = false;
					file_counts.comment_lines++;
				}
				else if (in_block_comment)
				{
					file_counts.comment_lines++;
				}
			}

			files[file_path.filename().string()] = file_counts;
			counts += file_counts;
		}
	}

	std::int64_t total_files() const { return files.size(); }
};

struct Solution 
{
	std::string name;
	std::filesystem::path path;
	std::map<std::string, Project> projects;

	Solution(std::string name) : name(std::move(name))
	{
	}

	void process_files()
	{
		for (auto&& project : projects)
		{
			project.second.process_files();
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

Solution parse_solution(std::string_view name, std::istream& file, const std::filesystem::path& parent_path, bool verbose);
Project parse_project(std::string_view name, std::istream&  file, const std::filesystem::path& solution_path, bool verbose);

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

Solution parse_solution(std::string_view name, std::istream& solution_file, const std::filesystem::path& solution_path, bool verbose)
{
	auto solution = Solution(std::string(name));
	std::string line;

	while (std::getline(solution_file, line))
	{
		if (line.starts_with("Project"))
		{
			// Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "LinesOfCode", "LinesOfCode.vcxproj", "{1FFDDF7A-0937-41D0-ACC7-FA78D4B60457}"

			std::string one;
			std::istringstream iss(line);
			std::getline(iss, one, '=');

			std::string project_name;
			std::string project_file;
			std::string project_uuid;
			std::getline(iss, project_name, ',');
			std::getline(iss, project_file, ',');
			std::getline(iss, project_uuid, ',');

			project_name = project_name.substr(2, project_name.size() - 3);
			project_file = project_file.substr(2, project_file.size() - 3);
			project_uuid = project_uuid.substr(3, project_uuid.size() - 5);

			//if (verbose) std::cout << "Found project: " << project_name << " in file: " << project_file << " with GUID: " << project_uuid << '\n';

			// read the filters file to see which files we need to count for this project

			std::filesystem::path project_path = project_file;
			if (!project_path.is_absolute())
			{
				project_path = solution_path.string() + "/" + project_file;
			}
			std::ifstream project_filters(project_path.string());

			if (!project_filters.is_open())
			{
				std::cerr << "Unable to open project at path: " << project_path.string() << std::endl;
				continue;
			}

			solution.projects[project_name] = parse_project(project_name, project_filters, project_path, verbose);
		}
	}
	return solution;
}

Project parse_project(std::string_view name, std::istream& project_filters, const std::filesystem::path& project_path, bool verbose)
{
	auto project = Project(std::string(name));
	std::string line;

	while (std::getline(project_filters, line))
	{
		if (line.contains("ClCompile Include") || line.contains("ClInclude Include"))
		{
			auto first = line.find_first_of('"');
			auto last = line.find_last_of('"');
			std::string file_name = line.substr(first + 1, last - first - 1);

			if (!file_name.ends_with(".h") && !file_name.ends_with(".cpp"))
			{
				continue;
			}

			auto path = std::filesystem::path(file_name);

			if (!path.is_absolute())
			{
				path = project_path.parent_path().string() + "/" + file_name;
			}
			project.file_paths.emplace_back(path);
		}
	}
	return project;
}