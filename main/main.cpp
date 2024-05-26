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
	if (argc < 3)
	{
		std::cerr << "Not enough arguments\n";
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

		OutputDetail outputDetail = OutputDetail::FILES;

		if (argc >= 5 && std::string(argv[4]) == "--detail-only-diffs")
		{
			outputDetail = OutputDetail::ONLY_DIFFS;
		}
		compare_solutions(argv[2], argv[3], OutputType::CONSOLE, outputDetail);
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
		single_solution(argv[2], OutputType::CONSOLE, OutputDetail::FILES);
	}
	else if (std::string(argv[1]) == "--single-project")
	{
		//parse_project()
	}
	else if (std::string(argv[1]) == "--cmake-project")
	{
		// using cmake-file-api v1 Client Stateless Query Files
		std::string build_directory = argv[2];
		std::string reply_directory = build_directory + "/.cmake/api/v1/reply/";

		// write query file to build directory
		std::filesystem::create_directories(build_directory + "/.cmake/api/v1/query/client-ccensus/");

		{
			std::ofstream(build_directory + "/.cmake/api/v1/query/client-ccensus/codemodel-v2");
		}

		// run cmake . in the build directory
		std::system(std::string("(cd " + build_directory + " && cmake .)").c_str());

		// remove the query file now that we're done with it
		std::filesystem::remove(build_directory + "/.cmake/api/v1/query/client-ccensus/codemodel-v2");

		std::vector<std::filesystem::directory_entry> index_files;

		// read reply json files from build directory
		for (auto const& dir_entry : std::filesystem::directory_iterator(build_directory + "/.cmake/api/v1/reply/")) 
		{
			if (dir_entry.path().filename().string().starts_with("index-"))
			{
				index_files.push_back(dir_entry);
			}
		}

		// TODO if there are no index files then something is wrong

		std::sort(index_files.begin(), index_files.end(), [](const auto& a, const auto& b) {
			return a.path().filename() < a.path().filename();
		});

		auto current_index_file = index_files.back();

		ondemand::parser parser;
		padded_string index_file_json = padded_string::load(current_index_file.path().string());

		// find our reply
		ondemand::document index = parser.iterate(index_file_json);

		auto model_file = index["reply"]["client-ccensus"]["codemodel-v2"]["jsonFile"];

		std::cout << model_file << '\n';

		std::string model_filepath = reply_directory + std::string(std::string_view(model_file));
		padded_string model_json = padded_string::load(model_filepath);

		ondemand::document model = parser.iterate(model_json);

		auto configurations = model["configurations"];

		for (auto config : configurations)
		{
			std::cout << "config: " << config["name"] << '\n';

			auto targets = config["targets"];

			for (auto target : targets)
			{
				std::cout << "target: " << target["name"] << '\n';
			}
		}
		
		auto version = model["version"];
		std::cout << "codemodel-v2: " << std::int64_t(version["major"]) <<
			'.' << std::int64_t(version["minor"]) << '\n';
	}

	auto clock_now = std::chrono::system_clock::now();
	auto elapsed_time = std::chrono::duration_cast <std::chrono::milliseconds> (clock_now - clock_start).count();
	std::cout << "\nElapsed Time:   " << elapsed_time << "ms \n";
}