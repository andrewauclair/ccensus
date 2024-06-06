#ifndef CCENSUS_DIFF_H
#define CCENSUS_DIFF_H

#include "backend.h"

#include <optional>
#include <map>

struct TargetDiff
{
    std::optional<Target> left;
    std::optional<Target> right;
};

struct ProjectDiff
{
    std::map<std::string, TargetDiff> differences;

    LineCounts total_line_count_diff() const
    {
        LineCounts a;
        LineCounts b;

        for (auto&& target : differences)
        {
            if (target.second.left)
            {
                a += target.second.left->total_counts();
            }
            if (target.second.right)
            {
                b += target.second.right->total_counts();
            }
        }

        return b - a;
    }
};

ProjectDiff project_difference(const std::string& json_a, const std::string& json_b);

#endif
