# ccensus

A command-line tool for counting lines of code in C and C++ projects. ccensus parses your build system directly using CMake or Visual Studio files and reports total lines, code lines, blank lines, and comment lines broken down by target and file.

## Features

- Supports **CMake** and **Visual Studio** (.sln) projects
- Distinguishes between **1st party** and **3rd party** targets
- Multiple output formats: console, JSON, and CSV
- **Diff mode** — compare two saved JSON snapshots to track how a codebase changes over time

## Building

ccensus requires CMake 3.22+ and a C++17-compatible compiler.

```sh
cmake -B build -S .
cmake --build build
```

The binary is placed in `build/bin/`.

## Usage

Exactly one of `--cmake`, `--visual-studio`, or `--diff` must be provided.

### CMake project

First configure your project's build directory, then point ccensus at it:

```sh
cmake -B build -S .
ccensus --cmake build --console
```

### Visual Studio solution

```sh
ccensus --visual-studio MyProject.sln --console
```

### Diff mode

Save snapshots of two builds as JSON, then compare them:

```sh
ccensus --cmake build-before --json --output-file before.json
ccensus --cmake build-after  --json --output-file after.json
ccensus --diff before.json after.json --console
```

## Options

| Flag | Description |
|---|---|
| `--cmake <dir>` | Read from a CMake build directory |
| `--visual-studio <file>` | Read from a Visual Studio `.sln` file |
| `--diff <a> <b>` | Compare two previously saved JSON output files |
| `--console` | Print results to the console (default) |
| `--json` | Write results to a JSON file |
| `--csv` | Write results to a CSV file |
| `--output-file <file>` | Set the output file name |
| `--targets-only` | Show per-target totals only, omitting individual files |
| `-v, --version` | Print version information |

## Output

Console output includes a summary table with total targets, files, and line counts split by 1st and 3rd party code, followed by a breakdown of the top 10 largest files in each category.

JSON output is a versioned format that can be passed back into ccensus later via `--diff` to compare two states of a codebase.

## Dependencies

- [CLI11](https://github.com/CLIUtils/CLI11) — command-line argument parsing
- [simdjson](https://github.com/simdjson/simdjson) — JSON parsing
- [nlohmann/json](https://github.com/nlohmann/json) — JSON output
