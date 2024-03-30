#define _CRT_SECURE_NO_WARNINGS // test

#include "project.h"
#include "solution.h"
#include "lib.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <span>
#include <algorithm>
#include <map>



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
// --detail <file | project>
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

		OutputDetail outputDetail = OutputDetail::Files;

		if (argc >= 5 && std::string(argv[4]) == "--detail-only-diffs")
		{
			outputDetail = OutputDetail::ONLY_DIFFS;
		}
		compare_solutions(argv[2], argv[3], OutputType::Console, outputDetail);
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
		single_solution(argv[2], OutputType::Console, OutputDetail::Files);
	}
	else if (std::string(argv[1]) == "--single-project")
	{
		//parse_project()
	}

	auto clock_now = std::chrono::system_clock::now();
	auto elapsed_time = std::chrono::duration_cast <std::chrono::milliseconds> (clock_now - clock_start).count();
	std::cout << "\nElapsed Time:   " << elapsed_time << "ms \n";
}