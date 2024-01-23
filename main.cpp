#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>

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
}

struct LineCounts
{
	std::int64_t total_lines = 0;
	std::int64_t code_only_lines = 0;
	std::int64_t code_and_comment_lines = 0;
	std::int64_t comment_lines = 0;
	std::int64_t blank_lines = 0;
};

struct Project 
{
	std::string name;
	std::filesystem::path path;
	std::vector<std::filesystem::path> files;

	LineCounts counts;

	Project(std::string name) : name(std::move(name))
	{
	}

	void process_files()
	{
		counts = {};
		bool in_block_comment = false;

		for (auto&& file_path : files)
		{
			auto file = std::ifstream(file_path);
			std::string line;

			while (std::getline(file, line))
			{
				counts.total_lines++;

				if (is_blank(line))
				{
					counts.blank_lines++;
				}

				if (is_single_comment(line))
				{
					counts.comment_lines++;
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
		}
	}

	std::int64_t total_files() const { return files.size(); }
};

struct Solution 
{
	std::string name;
	std::filesystem::path path;
	std::vector<Project> projects;

	Solution(std::string name) : name(std::move(name))
	{
	}

	void process_files()
	{
		for (auto&& project : projects)
		{
			project.process_files();
		}
	}

	std::int64_t total_projects() const { return projects.size(); }

	std::int64_t total_files() const
	{
		std::int64_t total = 0;
		for (auto&& project : projects) total += project.total_files();
		return total;
	}

	std::int64_t total_lines() const
	{
		std::int64_t total = 0;
		for (auto&& project : projects) total += project.counts.total_lines;
		return total;
	}

	std::int64_t blank_lines() const
	{
		std::int64_t total = 0;
		for (auto&& project : projects) total += project.counts.blank_lines;
		return total;
	}

	std::int64_t comment_lines() const
	{
		std::int64_t total = 0;
		for (auto&& project : projects) total += project.counts.comment_lines;
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
* some way to compare output. maybe an option where you can give it 2 folders, solutions or projects and compare the LOC differences
* care would have to be taken with files that exist in one and not the other
* 
* find a library that can parse command line args. I think I have one starred on GitHub
*/
// TODO project total lines should count projects that it depends on. The csv output will have a bunch of different pieces of info.
int main(int argc, char** argv)
{
	if (argc < 2)
	{
		return -1;
	}

	auto clock_start = std::chrono::system_clock::now();

	bool verbose = true;

	std::fstream solution_file(argv[1]);

	std::filesystem::path solution_path = std::filesystem::path(argv[1]).parent_path();

	auto solution = parse_solution(solution_path.filename().string(), solution_file, solution_path, verbose);
	
	solution.process_files();

	std::cout << "Solution " << solution.name << '\n';
	std::cout << "Total Projects: " << solution.total_projects() << '\n';
	std::cout << "Total Files:    " << solution.total_files() << '\n';
	std::cout << "Total Lines:    " << solution.total_lines() << '\n';
	std::cout << "Blank Lines:    " << solution.blank_lines() << '\n';
	std::cout << "Comment Lines:  " << solution.comment_lines() << '\n';

	auto clock_now = std::chrono::system_clock::now();
	auto elapsed_time = std::chrono::duration_cast <std::chrono::milliseconds> (clock_now - clock_start).count();
	std::cout << "Elapsed Time:   " << elapsed_time << "ms \n";
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

			if (verbose)
				std::cout << "Found project: " << project_name << " in file: " << project_file << " with GUID: " << project_uuid << '\n';

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

			solution.projects.emplace_back(parse_project(project_name, project_filters, project_path, verbose));
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
			project.files.emplace_back(path);
		}
	}
	return project;
}