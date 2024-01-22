#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		return -1;
	}

	auto clock_start = std::chrono::system_clock::now();

	bool verbose = false;

	std::fstream solution_file(argv[1]);
	std::string line;

	std::filesystem::path solution_path = std::filesystem::path(argv[1]).parent_path();	

	std::size_t total_lines_solution = 0;
	std::size_t total_projects = 0;
	std::size_t total_files_solution = 0;

	while (std::getline(solution_file, line))
	{
		if (line.starts_with("Project"))
		{
			// Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "LinesOfCode", "LinesOfCode.vcxproj", "{1FFDDF7A-0937-41D0-ACC7-FA78D4B60457}"
			//char guid[255]{};
			//char project_name[255]{};
			//char project_file[255]{};
			//char project_uuid[255]{};

			//std::sscanf(line.c_str(), "Project(\"{%s}\") = \"%s\", \"%s\",\"{%s}\"", guid, project_name, project_file, project_uuid);
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

			std::filesystem::path path = solution_path.string() + "/" + project_file;
			std::ifstream project_filters(path.string());

			if (!project_filters.is_open())
			{
				std::cerr << "Unable to open project at path: " << path.string() << std::endl;
				return -1;
			}

			total_projects++;

			std::size_t total_files = 0;
			std::size_t total_lines = 0;

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
					total_files++;
					total_files_solution++;

					// open the file and count the lines
					auto file_path = path.parent_path();
					file_path.append("/");
					file_path.append(file_name);
					std::ifstream file(path.parent_path().string() + "/" + file_name);

					if (!file.is_open())
					{
						std::cerr << "Unable to open file at path: " << (path.parent_path().string() + "/" + file_name) << std::endl;
						return -1;
					}

					std::string file_line;
					std::size_t count = 0;

					while (std::getline(file, file_line)) ++count;
					total_lines += count;
					total_lines_solution += count;
					if (verbose)
					std::cout << "Found file '" << file_name << "' with " << count << " lines of code\n";
				}
			}
			if (verbose)
			std::cout << "'" << project_file << "' total files: " << total_files << ", total lines : " << total_lines << '\n';
		}
	}

	std::cout << "Total Projects: " << total_projects << '\n';
	std::cout << "Total Files:    " << total_files_solution << '\n';
	std::cout << "Total Lines:    " << total_lines_solution << '\n';

	
	auto clock_now = std::chrono::system_clock::now();
	auto elapsed_time = std::chrono::duration_cast <std::chrono::milliseconds> (clock_now - clock_start).count();
	std::cout << "Elapsed Time:   " << elapsed_time << "ms \n";
}