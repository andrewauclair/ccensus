#include "project.h"

Project parse_project(const std::string& name, std::istream& project_filters, const std::filesystem::path& project_path)
{
	auto project = Project(name);
	std::string line;

	while (std::getline(project_filters, line))
	{
		if (line.contains("ClCompile Include") || line.contains("ClInclude Include"))
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
			project.file_paths.emplace_back(path);
		}
	}
	return project;
}