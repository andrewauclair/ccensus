#define _CRT_SECURE_NO_WARNINGS // test

#include "project.h"
#include "solution.h"
#include "lib.h"
#include "backend.h"
#include "cmake.h"

#include "CLI/CLI.hpp"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <span>
#include <algorithm>
#include <map>

#include "simdjson.h"
using namespace simdjson;

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
	auto app = CLI::App("Line of code counter");
	argv = app.ensure_utf8(argv);

	// --visual-studio
	// --cmake
	// --console (default)
	// --csv
	// --json
	// --diff <a.json> <b.json>
	// --targets-only
	std::string visual_studio_file;
	std::string cmake_build_dir;

	std::vector<std::string> json_diff;

	auto* vs = app.add_option("--visual-studio", visual_studio_file, "Read LOC data from a Visual Studio solution file (.sln)");
	vs->check(CLI::ExistingFile);

	auto* cmake = app.add_option("--cmake", cmake_build_dir, "Read LOC data from a cmake build directory");
	cmake->check(CLI::ExistingDirectory);
	
	auto* diff = app.add_option("--diff", json_diff, "Compare LOC data in two previously recorded JSON files");
	diff->check(CLI::ExistingFile);
	diff->expected(2);

	auto* group = app.add_option_group("types", "");
	group->add_option(vs);
	group->add_option(cmake);
	group->add_option(diff);

	// only allow one of --visual-studio, --cmake or --diff
	group->require_option(1);

	app.set_version_flag("-v,--version", "0.1 " __DATE__);
	
	bool console = false;
	bool csv = false;
	bool json = false;
	bool targets_only = false;

	app.add_flag("--console", console, "Output to the console");
	app.add_flag("--csv", csv, "Output to a CSV file");
	app.add_flag("--json", json, "Output to a JSON file");

	auto* targets_only_option = app.add_flag("--targets-only", targets_only, "Output only targets and not individual files");

	// --file-types <comma-separated-list> (default is .h,.hpp,.c,.cc,.cxx,.cpp)


	CLI11_PARSE(app, argc, argv);

	if (argc < 3)
	{
		std::cerr << "Not enough arguments\n";
		return -1;
	}

	auto clock_start = std::chrono::system_clock::now();

	bool verbose = true;

	if (!visual_studio_file.empty())
	{
		single_solution(visual_studio_file, OutputType::CONSOLE, OutputDetail::FILES);
	}
	else if (!cmake_build_dir.empty())
	{
		auto cmake = CMakeFrontend(cmake_build_dir);

		Package package = cmake.parse();

		package.process();

		Backend backend;
		backend.generate_info_output(package, OutputType::CONSOLE, targets_only);
	}

	auto clock_now = std::chrono::system_clock::now();
	auto elapsed_time = std::chrono::duration_cast <std::chrono::milliseconds> (clock_now - clock_start).count();
	std::cout << "\nElapsed Time:   " << elapsed_time << "ms \n";
}