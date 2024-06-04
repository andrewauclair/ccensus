#include "diff.h"

#include <simdjson.h>

ProjectDiff project_difference(const std::string& json_a, const std::string& json_b)
{
    ProjectDiff diffs;

    simdjson::ondemand::parser parser;

    simdjson::padded_string string_a = simdjson::padded_string::load(json_a);
    simdjson::padded_string string_b = simdjson::padded_string::load(json_b);

    simdjson::ondemand::document doc_a = parser.iterate(string_a);
    simdjson::ondemand::document doc_b = parser.iterate(string_b);

    // TODO check the format version of the file
    const auto load_document = [&](simdjson::ondemand::document& doc, bool left) {
        for (auto json : doc.get_object())
        {
            if (json.key() == "info")
            {
            }
            else if (json.key() == "targets")
            {
                for (auto target : json.value().get_array())
                {
                    Target load_target;

                    auto files = target["files"];

                    for (auto file : files.get_array())
                    {
                        auto lines = file["lines"];

                        LineCounts counts;

                        int i = 0;
                        for (auto count : lines)
                        {
                            switch (i)
                            {
                                case 0:
                                counts.total_lines = count;
                                break;
                                case 1:
                                counts.code_only_lines = count;
                                break;
                                case 2:
                                counts.code_and_comment_lines = count;
                                break;
                                case 3:
                                counts.comment_lines = count;
                                break;
                                case 4:
                                counts.blank_lines = count;
                                break;
                            }
                            i++;
                        }
                        // counts.total_lines = lines.at(0);
                        // counts.code_only_lines = lines.at(1);
                        // counts.code_and_comment_lines = lines.at(2);
                        // counts.comment_lines = lines.at(3);
                        // counts.blank_lines = lines.at(4);

                        std::string name = std::string(std::string_view(file["name"]));
                        load_target.file_counts[name] = counts;
                        load_target.files.push_back(name);

                        
                    }
load_target.name = static_cast<std::string_view>(target["name"]);
                    if (left)
                    {
                        diffs.differences[load_target.name].left = load_target;
                    }
                    else
                    {
                        diffs.differences[load_target.name].right = load_target;
                    }
                }
            }
        }
    };

    load_document(doc_a, true);
    load_document(doc_b, false);

    return diffs;
}