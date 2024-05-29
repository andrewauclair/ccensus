#ifndef CCENSUS_DISPLAY_UTILS_H
#define CCENSUS_DISPLAY_UTILS_H

#include <cstdint>
#include <ostream>
#include <variant>
#include <string>
#include <format>

inline std::int64_t width_for_value(std::int64_t value)
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

#endif
