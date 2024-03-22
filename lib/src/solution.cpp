#include "solution.h"

#include <fstream>
#include <iostream>

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