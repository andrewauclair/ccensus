#ifndef CCENSUS_BACKEND_H
#define CCENSUS_BACKEND_H

#include "utils.h"
#include "data.h"

enum class OutputType
{
    CONSOLE,
    CSV,
    JSON
};

class Backend
{
public:
    void generate_info_output(const Package& package, OutputType outputType, bool targets_only);
    void generate_diff_output(const Package& package, OutputType outputType, bool targets_only);
    
private:
    void generate_info_console(const Package& package);
    void generate_info_csv(const Package& package);
    void generate_info_json(const Package& package);

    void generate_diff_console(const Package& package);
    void generate_diff_csv(const Package& package);
    void generate_diff_json(const Package& package);

private:
    bool m_targets_only = false;
};

#endif
