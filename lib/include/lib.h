#ifndef LIB_H
#define LIB_H

#include <string>

enum OutputType
{
	Console,
	CSV
};

enum OutputDetail
{
	Files,
	Projects,
	ONLY_DIFFS
};

void compare_solutions(const std::string& solution_a, const std::string& solution_b, OutputType outputType, OutputDetail outputDetail);
void single_solution(const std::string& solution_name, OutputType outputType, OutputDetail outputDetail);
void single_project(const std::string& project, OutputType outputType, OutputDetail outputDetail);

#endif