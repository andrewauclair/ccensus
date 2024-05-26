#include "lib.h"
#include "solution.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <ranges>
#include <variant>
#include <array>
#include <format>

struct FileDiffs {
	std::optional<LineCounts> before;
	std::optional<LineCounts> after;
};

struct ProjectDiffs {
	std::map<std::string, FileDiffs> diffs;

	std::optional<LineCounts> before_totals() const
	{
		LineCounts counts;
		bool has_any = false;
		for (auto&& diffs : std::views::values(diffs))
		{
			if (diffs.before.has_value()) has_any = true;
			counts += diffs.before.value_or(LineCounts());
		}
		if (!has_any)
		{
			return std::nullopt;
		}
		return counts;
	}

	std::optional<LineCounts> after_totals() const
	{
		LineCounts counts;
		bool has_any = false;
		for (auto&& diffs : std::views::values(diffs))
		{
			if (diffs.after.has_value()) has_any = true;
			counts += diffs.after.value_or(LineCounts());
		}
		if (!has_any)
		{
			return std::nullopt;
		}
		return counts;
	}
};

std::int64_t width_for_value(std::int64_t value)
{
	std::int64_t width = 1;
	std::int64_t temp = 1;

	if (value < 0)
	{
		width++;
		value = std::abs(value);
	}

	while (temp < value)
	{
		temp *= 10;
		width++;
	}
	return width;
}

template<std::size_t Size>
struct Row {
	std::array<std::variant<std::string, std::int64_t>, Size> cells{};
};

template<std::size_t Size>
struct Table {
	std::vector<Row<Size>> rows;
	std::array<std::int64_t, Size> minimum_column_widths{};
	std::array<bool, Size> set_showpos{};

	template<typename T, typename... Args>
	void insert_row(T value, Args... values)
	{
		auto& row = rows.emplace_back();
		row.cells = { value, values... };
	}

	std::array<std::int64_t, Size> calculate_column_widths() const
	{
		std::array<std::int64_t, Size> widths = minimum_column_widths;

		for (auto&& row : rows)
		{
			std::size_t column = 0;
			std::int64_t width = 0;

			for (auto&& cell : row.cells)
			{
				std::visit([&width](auto&& arg)
					{
						using T = std::decay_t<decltype(arg)>;

						if constexpr (std::is_same_v<T, std::string>)
						{
							width = arg.length();
						}
						else if constexpr (std::is_same_v<T, std::int64_t>)
						{
							width = width_for_value(arg);
						}
					}, cell);

				widths[column] = std::max(widths[column], width + 3);
				column++;
			}
		}

		return widths;
	}

	friend std::ostream& operator<<(std::ostream& out, const Table<Size>& table)
	{
		const auto widths = table.calculate_column_widths();

		for (auto&& row : table.rows)
		{
			std::size_t column = 0;

			for (auto&& cell : row.cells)
			{
				std::visit([&](auto&& arg)
					{
						if (column == 0)
						{
							out << std::left;
						}
						else
						{
							out << std::right;
						}
						out << std::setw(widths[column]);
						
						using T = std::decay_t<decltype(arg)>;

						if constexpr (std::is_same_v<T, std::int64_t>)
						{
							if (table.set_showpos[column])
							{
								out << std::format(std::locale("en_US.UTF-8"), "{:+L}", arg);
							}
							else
							{
								out << std::format(std::locale("en_US.UTF-8"), "{:L}", arg);
							}
						}
						else
						{
							out << arg;
						}
					}, cell);
				column++;
			}
			out << '\n';
		}
		return out;
	}
};

void compare_solutions(const std::string& solution_a, const std::string& solution_b, OutputType outputType, OutputDetail outputDetail)
{
	std::ifstream solutionA_file(solution_a);
	std::ifstream solutionB_file(solution_b);

	std::filesystem::path solutionA_path = std::filesystem::path(solution_a).parent_path();
	std::filesystem::path solutionB_path = std::filesystem::path(solution_b).parent_path();

	auto solutionA = parse_solution(solutionA_path.filename().string(), solutionA_file, solutionA_path);
	auto solutionB = parse_solution(solutionB_path.filename().string(), solutionB_file, solutionB_path);

	solutionA.process_files();
	solutionB.process_files();

	//						A			B		Difference
	// Total Projects		2			3			+1
	// Total Files:
	// Total Lines:
	// Blank Lines:
	// Comment Lines:

	std::cout << "Before = " << solutionA_path << '\n';
	std::cout << "After  = " << solutionB_path << '\n';
	std::cout << "\n\n";

	Table<4> summary;
	summary.set_showpos[3] = true; // show difference with std::showpos
	summary.insert_row("", "Before", "After", "Difference");
	summary.insert_row("Projects", solutionA.total_projects(), solutionB.total_projects(), solutionB.total_projects() - solutionA.total_projects());
	summary.insert_row("Files", solutionA.total_files(), solutionB.total_files(), solutionB.total_files() - solutionA.total_files());
	summary.insert_row("Total Lines", solutionA.total_lines(), solutionB.total_lines(), solutionB.total_lines() - solutionA.total_lines());
	summary.insert_row("Physical Lines", solutionA.physical_lines(), solutionB.physical_lines(), solutionB.physical_lines() - solutionA.physical_lines());
	summary.insert_row("", "", "", "");
	summary.insert_row("", "", "", "");
	summary.insert_row("", "", "", "");

	std::map<std::string, ProjectDiffs> project_diffs;

	for (auto&& project : solutionA.projects_view())
	{
		auto& diffs = project_diffs[project.first];

		for (auto&& [file, counts] : project.second.files_view())
		{
			auto& file_diffs = diffs.diffs[file];

			file_diffs.before = counts;
		}
	}

	for (auto&& project : solutionB.projects_view())
	{
		auto& diffs = project_diffs[project.first];

		for (auto&& [file, counts] : project.second.files_view())
		{
			auto& file_diffs = diffs.diffs[file];

			file_diffs.after = counts;
		}
	}

	const auto build_row = [&summary, &outputDetail](const std::string& name, std::optional<LineCounts> before, std::optional<LineCounts> after)
		{
			std::variant<std::string, std::int64_t> before_lines = "";
			std::variant<std::string, std::int64_t> after_lines = "";
			std::int64_t total_diff = 0;

			if (before) before_lines = before.value().total_lines;
			if (after) after_lines = after.value().total_lines;
			if (before && after)
			{
				total_diff = after.value().total_lines - before.value().total_lines;
			}
			else if (before)
			{
				total_diff = -before.value().total_lines;
			}
			else if (after)
			{
				total_diff = after.value().total_lines;
			}
			std::variant<std::string, std::int64_t> diff = total_diff;
			if (total_diff == 0) diff = "";

			if (outputDetail != OutputDetail::ONLY_DIFFS || total_diff != 0)
			{
				summary.insert_row(name, before_lines, after_lines, diff);
			}
		};

	for (auto&& [project, diffs] : project_diffs)
	{
		build_row(project, diffs.before_totals(), diffs.after_totals());
		summary.insert_row("", "", "", "");

		for (auto&& [file, file_diffs] : diffs.diffs)
		{
			build_row(file, file_diffs.before, file_diffs.after);
		}
		summary.insert_row("", "", "", "");
		summary.insert_row("", "", "", "");
	}

	std::cout << summary;
	std::cout << "\n\n\n";
}

void single_solution(const std::string& solution_name, OutputType outputType, OutputDetail outputDetail)
{
	std::ifstream solution_file(solution_name);

	std::filesystem::path solution_path = std::filesystem::path(solution_name).parent_path();

	auto solution = parse_solution(solution_path.filename().string(), solution_file, solution_path);

	solution.process_files();

	std::cout << "Solution " << solution.name << "\n\n";
	
	Table<2> summary;
	summary.insert_row("Total Projects", solution.total_projects());
	summary.insert_row("Total Files", solution.total_files());
	summary.insert_row("Total Lines", solution.total_lines());

	std::cout << summary << "\n\n\n";

	for (auto&& project : std::views::values(solution.projects))
	{
		if (project.files.empty())
		{
			continue;
		}
		std::cout << "Project: " << project.name << "\n\n";

		Table<3> project_out;
		project_out.insert_row("File Name", "TLOC", "PLOC");

		for (auto&& [file_name, file_counts] : project.files)
		{
			project_out.insert_row(file_name, file_counts.total_lines, file_counts.physical_lines());
		}
		std::cout << project_out << "\n\n\n";
	}
}

void single_project(const std::string& project, OutputType outputType, OutputDetail outputDetail)
{

}