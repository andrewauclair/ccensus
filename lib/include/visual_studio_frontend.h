#ifndef CCENSUS_VISUAL_STUDIO_FRONTEND_H
#define CCENSUS_VISUAL_STUDIO_FRONTEND_H

#include "data.h"

#include <string>

class VisualStudioFrontend
{
public:
	VisualStudioFrontend(const std::string& visual_studio_sln);

	Package parse();

private:
	Package parse_solution(const std::string& name, std::istream& solution_file, const std::filesystem::path& solution_path);
	Target parse_project(const std::string& name, std::istream& project_filters, const std::filesystem::path& project_path);

	std::string m_visual_studio_sln;
};

#endif
