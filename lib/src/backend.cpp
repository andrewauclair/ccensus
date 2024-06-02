#include "backend.h"
#include "display_utils.h"

#include <iostream>
#include <algorithm>
#include <string>
#include <set>

#include <nlohmann/json.h>

void Backend::generate_info_output(const Package& package, OutputType outputType, bool targets_only)
{
    m_targets_only = targets_only;

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

void Backend::generate_diff_output(const Package& package, OutputType outputType, bool targets_only)
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
        total_files += target.files.size();

        if (target.is_third_party)
        {
            total_3rd_party_files += target.files.size();
        }
        else
        {
            total_1st_party_files += target.files.size();
        }
    }

    std::cout.imbue(std::locale(""));

    std::cout << "\n\n";
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

    // print each of the 1st party targets
    for (const Target& target : package.targets)
    {
        if (target.is_third_party)
        {
            continue;
        }
        std::cout << '\n' << "1st Party Target Totals: " << target.name << "\n\n";
        std::cout << "Total Files: " << target.files.size() << '\n';
        std::cout << "Total Lines: " << target.total_counts.total_lines << '\n';
        std::cout << "Total Physical Lines: " << target.total_counts.physical_lines() << '\n';
        std::cout << "\n\n";
    }

    // print each of the 3rd party targets
    for (const Target& target : package.targets)
    {
        if (!target.is_third_party)
        {
            continue;
        }
        std::cout << '\n' << "3rd Party Target Totals: " << target.name << "\n\n";
        std::cout << "Total Files: " << target.files.size() << '\n';
        std::cout << "Total Lines: " << target.total_counts.total_lines << '\n';
        std::cout << "Total Physical Lines: " << target.total_counts.physical_lines() << '\n';
        std::cout << "\n\n";
    }

    const auto print_target_files = [](const Target& target) {
        std::vector<std::string> files;
        files.insert(files.end(), target.files.begin(), target.files.end());

        for (auto&& path : target.include_paths)
        {
            if (!target.is_node_target_include_directory(path.backtrace_index))
            {
                continue;
            }
            
            try
            {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(path.path))
                {
                    if (entry.is_directory())
                    {
                        continue;
                    }
                    
                    const auto path = entry.path().string();

                    if (path.ends_with(".h") || path.ends_with(".hpp") ||
                        path.ends_with(".c") || path.ends_with(".cc") || path.ends_with(".cxx") || path.ends_with(".cpp"))
                    {
                        files.push_back(entry.path().string());
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Failed to read from directory " << path.path << "because: " << e.what() << '\n';
            }
        }
        
        std::set<std::string> file_set;
        file_set.insert(files.begin(), files.end());
        files.clear();
        files.insert(files.end(), file_set.begin(), file_set.end());

        std::sort(files.begin(), files.end(), [&target](const std::string& file_a, const std::string& file_b) {
            return target.file_counts.at(file_a).total_lines > target.file_counts.at(file_b).total_lines;
        });

        Table<3> output;
        output.insert_row("File Name", "TLOC", "PLOC");

        for (const std::string& file : files)
        {
            output.insert_row(file, target.file_counts.at(file).total_lines, target.file_counts.at(file).physical_lines());
        }
        std::cout << output;
    };

    if (!m_targets_only)
    {
        // print each of the 1st party targets
        for (const Target& target : package.targets)
        {
            if (target.is_third_party)
            {
                continue;
            }
            std::cout << '\n' << "1st Party Target Files: " << target.name << "\n\n";

            print_target_files(target);
        }

        // print each of the 3rd party targets
        for (const Target& target : package.targets)
        {
            if (!target.is_third_party)
            {
                continue;
            }
            std::cout << '\n' << "3rd Party Target Files: " << target.name << "\n\n";

            print_target_files(target);
        }

        

        std::map<int, std::string> firstPartyFiles;
        std::map<int, std::string> thirdPartyFiles;

        for (const Target& target : package.targets)
        {
            if (target.is_third_party)
            {
                for (auto&& file : target.files)
                {
                    thirdPartyFiles[target.file_counts.at(file).total_lines] = file;
                }
            }
            else
            {
                for (auto&& file : target.files)
                {
                    firstPartyFiles[target.file_counts.at(file).total_lines] = file;
                }
            }
        }

        std::cout << "\n\n";
        std::cout << "Top 10 Largest 1st Party Files:\n\n";

        int i = 0;

        Table<2> firstPartyFileTable;
        firstPartyFileTable.insert_row("File", "Total Lines");

        for (auto it = firstPartyFiles.rbegin(); it != firstPartyFiles.rend() && i < 10; ++it, ++i)
        {
            firstPartyFileTable.insert_row(it->second, it->first);
        }

        std::cout << firstPartyFileTable;
        std::cout << "\n\n";
        std::cout << "Top 10 Largest 3rd Party Files:\n\n";

        Table<2> thirdPartyFileTable;
        thirdPartyFileTable.insert_row("File", "Total Lines");

        i = 0;

        for (auto it = thirdPartyFiles.rbegin(); it != thirdPartyFiles.rend() && i < 10; ++it, ++i)
        {
            thirdPartyFileTable.insert_row(it->second, it->first);
        }

        std::cout << thirdPartyFileTable;
    }
}

void Backend::generate_info_csv(const Package& package)
{
    
}

void Backend::generate_info_json(const Package& package)
{
    using nlohman_json;
    // info: ccensus version, file format
    // per target: id, name, path, files, 3rd party flag, list of ids of dependencies
    // per file: full path, total lines, physical lines, blank lines, comment lines
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