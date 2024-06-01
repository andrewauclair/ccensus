#include "visual_studio_frontend.h"
#include <fstream>
#include <iostream>

VisualStudioFrontend::VisualStudioFrontend(const std::string& visual_studio_sln)
	: m_visual_studio_sln(visual_studio_sln)
{
}

Package VisualStudioFrontend::parse()
{
	std::ifstream solution_file(m_visual_studio_sln);

	std::filesystem::path solution_path = std::filesystem::path(m_visual_studio_sln).parent_path();

	return parse_solution(solution_path.filename().string(), solution_file, solution_path);
}

Package VisualStudioFrontend::parse_solution(const std::string& name, std::istream& solution_file, const std::filesystem::path& solution_path)
{
	Package package;

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

			// remove quotes
			project_name = project_name.substr(2, project_name.size() - 3);
			project_file = project_file.substr(2, project_file.size() - 3);
			project_uuid = project_uuid.substr(3, project_uuid.size() - 5);

			// read the filters file to see which files we need to count for this project

			std::filesystem::path project_path = project_file;
			if (!project_path.is_absolute())
			{
				project_path = solution_path.string() + "/" + project_file;
			}
			std::ifstream project_filters(project_path.string());

			if (!project_filters.is_open())
			{
				std::cerr << "Unable to open project at path: " << project_file << std::endl;
				continue;
			}

			package.targets.push_back(parse_project(project_name, project_filters, project_file));
		}
	}
	return package;
}

Target VisualStudioFrontend::parse_project(const std::string& name, std::istream& project_filters, const std::filesystem::path& project_path)
{
	Target target;
	target.id = name;
	target.name = name;

	std::string line;

	while (std::getline(project_filters, line))
	{
		if (line.contains("ClCompile Include") || line.contains("ClInclude Include") ||
			line.contains("None Include"))
		{
			auto first = line.find_first_of('"');
			auto last = line.find_last_of('"');
			std::string file_name = line.substr(first + 1, last - first - 1);

			auto path = std::filesystem::path(file_name);

			auto ext = path.extension();

			if (ext != ".h" && ext != ".hpp" && ext != ".c" && ext != ".cpp" && ext != ".H" && ext != ".CPP" && ext != ".C")
			{
				continue;
			}

			if (!path.is_absolute())
			{
				path = project_path.parent_path().string() + "/" + file_name;
			}

			target.files.push_back(file_name);
		}
	}
	return target;
}
