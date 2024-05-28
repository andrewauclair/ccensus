#include "backend.h"
#include <iostream>
#include <algorithm>

void Backend::generate_info_output(const Package& package, OutputType outputType)
{
    switch (outputType)
    {
    case OutputType::CONSOLE:
        generate_info_console(package);
        break;
    case OutputType::CSV:
        generate_info_csv(package);
        break;
    case OutputType::JSON:
        generate_info_json(package);
        break;
    }
}

void Backend::generate_diff_output(const Package& package, OutputType outputType)
{
    switch (outputType)
    {
    case OutputType::CONSOLE:
        generate_diff_console(package);
        break;
    case OutputType::CSV:
        generate_diff_csv(package);
        break;
    case OutputType::JSON:
        generate_diff_json(package);
        break;
    }
}

void Backend::generate_info_console(const Package& package)
{
    // Total Targets: <count>
    // 1st Party Targets: <count>
    // 3rd Party Targets: <count>
    //
    // Total Files: <count>
    // Total Lines: <count>
    //
    // Target: <target-name>
    //
    // File Name        TLOC        PLOC
    //
    // 
    
    LineCounts total_lines;
    LineCounts total_1st_party_lines;
    LineCounts total_3rd_party_lines;

    for (const Target& target : package.targets)
    {
        total_lines += target.total_counts;

        if (target.is_third_party)
        {
            total_3rd_party_lines += target.total_counts;
        }
        else
        {
            total_1st_party_lines += target.total_counts;
        }
    }

    std::int64_t total_files = 0;
    std::int64_t total_1st_party_files = 0;
    std::int64_t total_3rd_party_files = 0;

    for (const Target& target : package.targets)
    {
        total_files += target.include_files.size();
        total_files += target.source_files.size();

        if (target.is_third_party)
        {
            total_3rd_party_files += target.include_files.size();
            total_3rd_party_files += target.source_files.size();
        }
        else
        {
            total_1st_party_files += target.include_files.size();
            total_1st_party_files += target.source_files.size();
        }
    }

    std::cout << "Total Targets: " << package.targets.size() << '\n';
    std::cout << "Total Files: " << total_files << '\n';
    std::cout << "Total Lines: " << total_lines.total_lines << '\n';
    std::cout << "Total Physical Lines: " << total_lines.physical_lines() << '\n';
    std::cout << '\n';

    const auto third_party_target_count = std::count_if(package.targets.begin(), package.targets.end(), 
    [](const Target& target) {
        return target.is_third_party;
    });

    std::cout << "Total 1st Party Targets: " << (package.targets.size() - third_party_target_count) << '\n';
    std::cout << "Total 1st Party Files: " << total_1st_party_files << '\n';
    std::cout << "Total 1st Party Lines: " << total_1st_party_lines.total_lines << '\n';
    std::cout << "Total 1st Party Physical Lines: " << total_1st_party_lines.physical_lines() << '\n';
    std::cout << '\n';

    std::cout << "Total 3rd Party Targets: " << third_party_target_count << '\n';
    std::cout << "Total 3rd Party Files: " << total_3rd_party_files << '\n';
    std::cout << "Total 3rd Party Lines: " << total_3rd_party_lines.total_lines << '\n';
    std::cout << "Total 3rd Party Physical Lines: " << total_3rd_party_lines.physical_lines() << '\n';

    std::cout << '\n';

    
}

void Backend::generate_info_csv(const Package& package)
{
    
}

void Backend::generate_info_json(const Package& package)
{
    
}

void Backend::generate_diff_console(const Package& package)
{
    
}

void Backend::generate_diff_csv(const Package& package)
{
    
}

void Backend::generate_diff_json(const Package& package)
{

}