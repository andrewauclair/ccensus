#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>

struct Project 
{
	std::string name;
	std::filesystem::path path;
	std::vector<std::filesystem::path> files;

	Project(std::string name) : name(std::move(name))
	{
	}

	std::int64_t total_files() const { return files.size(); }

	std::int64_t total_lines() const
	{
		std::int64_t total = 0;
		for (auto&& file_path : files)
		{
			auto file = std::ifstream(file_path);
			std::string line;

			while (std::getline(file, line))
			{
				total++;
			}
		}
		return total;
	}
};

struct Solution 
{
	std::string name;
	std::filesystem::path path;
	std::vector<Project> projects;

	Solution(std::string name) : name(std::move(name))
	{
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
		for (auto&& project : projects) total += project.total_lines();
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

	const auto solution = parse_solution(solution_path.filename().string(), solution_file, solution_path, verbose);
	
	std::cout << "Total Projects: " << solution.total_projects() << '\n';
	std::cout << "Total Files:    " << solution.total_files() << '\n';
	std::cout << "Total Lines:    " << solution.total_lines() << '\n';

	
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