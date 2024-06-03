#define _CRT_SECURE_NO_WARNINGS // test

#include "backend.h"
#include "cmake.h"
#include "visual_studio_frontend.h"

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

struct CommandLineOptions
{
	std::string visual_studio_file;
	std::string cmake_build_dir;
	std::vector<std::string> json_diff;

	std::string output_file;

	// flags
	bool console = false;
	bool csv = false;
	bool json = false;
	bool targets_only = false;

	OutputType outputType() const
	{
		if (csv) return OutputType::CSV;
		if (json) return OutputType::JSON;
		return OutputType::CONSOLE;
	}
};

void configure_and_parse_cli(CommandLineOptions& options, int argc, char** argv)
{
	auto app = CLI::App("Line of code counter");
	app.set_version_flag("-v,--version", "0.1 " __DATE__);

	app.add_flag("--console", options.console, "Output to the console");
	app.add_flag("--csv", options.csv, "Output to a CSV file");
	app.add_flag("--json", options.json, "Output to a JSON file");
	app.add_flag("--targets-only", options.targets_only, "Output only targets and not individual files");

	argv = app.ensure_utf8(argv);

	auto* vs = app.add_option("--visual-studio", options.visual_studio_file, "Read LOC data from a Visual Studio solution file (.sln)");
	auto* cmake = app.add_option("--cmake", options.cmake_build_dir, "Read LOC data from a cmake build directory");
	auto* diff = app.add_option("--diff", options.json_diff, "Compare LOC data in two previously recorded JSON files");

	app.add_option("--output-file", options.output_file, "The file to output results to");

	auto* group = app.add_option_group("types", "");
	group->add_option(vs);
	group->add_option(cmake);
	group->add_option(diff);

	// only allow one of --visual-studio, --cmake or --diff
	group->require_option(1);

	// parse ourselves instead of using CLI11_PARSE
	try
	{
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& e)
	{
		std::exit(app.exit(e));
	}
}

int main(int argc, char** argv)
{
	CommandLineOptions options;
	configure_and_parse_cli(options, argc, argv);

	auto clock_start = std::chrono::system_clock::now();

	if (!options.visual_studio_file.empty() || !options.cmake_build_dir.empty())
	{
		Package package;

		if (!options.visual_studio_file.empty())
		{
			auto vs = VisualStudioFrontend(options.visual_studio_file);

			package = vs.parse();
			package.source_dir = std::filesystem::path(options.visual_studio_file).parent_path().string();
		}
		else if (!options.cmake_build_dir.empty())
		{
			auto cmake = CMakeFrontend(options.cmake_build_dir);

			package = cmake.parse();
		}

std::cout << package.source_dir << '\n';

		package.output_file = options.output_file;
		
		package.process();

		Backend backend;
		backend.generate_info_output(package, options.outputType(), options.targets_only);
	}

	auto clock_now = std::chrono::system_clock::now();
	auto elapsed_time = std::chrono::duration_cast <std::chrono::milliseconds> (clock_now - clock_start).count();
	std::cout << "\nElapsed Time:   " << elapsed_time << "ms \n";
}