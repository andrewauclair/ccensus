#include "project.h"

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