# gits

`gits` is a high-performance, local-first C++23 command-line utility designed to recursively scan directory trees, discover Git repositories, and provide either full status reports or quick condensed summaries.

## Features

* **Recursive Discovery**: Scans target directories efficiently for `.git` folders.
* **Smart Pruning**: Automatically skips heavy or irrelevant subdirectories (`build`, `vcpkg`, `node_modules`, `.vs`, etc.) for lightning-fast scans.
* **Short Summary Mode (`-s`)**: Lists all discovered repositories with a clean, color-coded one-line status summary (`git status -sb`).
* **Index Navigation**: Allows targeting specific repositories by their discovery index (`gits <path> <N>`) to fetch their absolute paths or automate workflows.
* **Unicode Resilient**: Fully safe path-handling on Windows to prevent multi-byte character mapping crashes.
* **Terminal Colored**: Enhanced visual output using ANSI colors via `rang`.

## Usage

```powershell
# Show full status reports for all repositories recursively
gits <parent-folder-path>

# Show a quick condensed short summary (-sb) for all repositories
gits <parent-folder-path> -s

# Output the absolute path of the N-th discovered repository
gits <parent-folder-path> <repository-index>
