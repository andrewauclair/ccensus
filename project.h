#ifndef LOC_PROJECT_H
#define LOC_PROJECT_H

#include "utils.h"

#include <string>
#include <filesystem>
#include <map>
#include <fstream>

struct LineCounts
{
	std::int64_t total_lines = 0;
	std::int64_t code_only_lines = 0;
	std::int64_t code_and_comment_lines = 0;
	std::int64_t comment_lines = 0;
	std::int64_t blank_lines = 0;

	void operator+=(const LineCounts& counts)
	{
		total_lines += counts.total_lines;
		code_only_lines += counts.code_only_lines;
		code_and_comment_lines += counts.code_and_comment_lines;
		comment_lines += counts.comment_lines;
		blank_lines += counts.blank_lines;
	}
};

struct Project
{
	std::string name;
	std::filesystem::path path;
	std::vector<std::filesystem::path> file_paths;
	std::map<std::string, LineCounts> files;

	LineCounts counts;

	friend bool operator<(const Project& lhs, const Project& rhs)
	{
		return lhs.name < rhs.name;
	}

	Project() = default;

	Project(std::string name) : name(std::move(name))
	{
	}

	const std::map<std::string, LineCounts>& files_view() const
	{
		return files;
	}

	void process_files()
	{
		std::sort(file_paths.begin(), file_paths.end());

		counts = {};
		bool in_block_comment = false;

		for (auto&& file_path : file_paths)
		{
			auto file = std::ifstream(file_path);
			std::string line;

			LineCounts file_counts;

			while (std::getline(file, line))
			{
				file_counts.total_lines++;

				if (!in_block_comment)
				{
					if (is_blank(line))
					{
						file_counts.blank_lines++;
					}

					if (is_single_comment(line))
					{
						file_counts.comment_lines++;
					}
				}

				if (!in_block_comment && is_opening_block_comment(line) == Block_Comment_Type::Opening)
				{
					// TODO need to check if there was any code on this line too
					in_block_comment = true;
					file_counts.comment_lines++;
				}
				else if (in_block_comment && is_closing_block_comment(line))
				{
					in_block_comment = false;
					file_counts.comment_lines++;
				}
				else if (in_block_comment)
				{
					file_counts.comment_lines++;
				}
			}

			files[file_path.filename().string()] = file_counts;
			counts += file_counts;
		}
	}

	std::int64_t total_files() const { return files.size(); }
};

Project parse_project(std::string_view name, std::istream& file, const std::filesystem::path& solution_path, bool verbose);

#endif
