# gits

`gits` is a lightweight, high-performance command-line utility written in **C++23** that recursively scans a specified parent directory for `.git` folders and executes `git status` on every discovered repository. It utilizes [rang](https://github.com/agauniyal/rang) for clean, colorful terminal output.

## Features

* **Recursive Directory Scanning:** Automatically traverses subfolders under a given root path.
* **Resilient Iteration:** Employs safe filesystem error-handling and skips permission-denied directories to prevent crashes on restricted system paths.
* **Color-Coded Output:** Highlights repository discoveries, status headers, and errors using terminal escape sequences via `rang`.
* **Cross-Platform Compatibility:** Built using modern C++23 features and standard `std::filesystem`.

---

## Prerequisites

* A C++ compiler supporting C++23 (e.g., Clang 16+, MSVC 19.30+, or GCC 13+).
* **CMake** (version 3.25 or higher).
* **Ninja** build system.

---

## Getting Started & Build Instructions

### 1. Clone with Submodules
Clone the repository along with its `rang` submodule:

```bash
git clone --recurse-submodules [https://github.com/yourusername/CppGitStatus.git](https://github.com/yourusername/CppGitStatus.git)
cd CppGitStatus
