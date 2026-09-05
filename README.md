# gits

`gits` is a lightweight, high-performance command-line utility written in **C++23** that recursively scans a specified parent directory for `.git` folders and executes `git status` on every discovered repository. It utilizes [rang](https://github.com/agauniyal/rang) for clean, colorful terminal output.

## Features

* **Recursive Directory Scanning:** Automatically traverses subfolders under a given root path.
* **Resilient Iteration:** Employs safe filesystem error-handling and skips permission-denied directories to prevent crashes on restricted system paths.
* **Color-Coded Output:** Highlights repository discoveries, status headers, and errors using terminal escape sequences via `rang`.
* **Cross-Platform Compatibility:** Built using modern C++23 features and standard `std::filesystem`.

---

## Architecture & Workflow

The diagram below illustrates how `gits` navigates a directory tree and interacts with discovered repositories:

```mermaid
flowchart TD
    Start([Start gits <path>]) --> Validate{Valid Directory?}
    Validate -- No --> Err1[Show Red Error & Exit]
    Validate -- Yes --> Scan[Initialize Recursive Iterator]
    
    Scan --> Loop{More Entries?}
    Loop -- No --> Finish[Display Summary & Exit]
    
    Loop -- Yes --> Read[Read Next Path Entry]
    Read --> CheckErr{Error / Access Denied?}
    CheckErr -- Yes --> Skip[Skip Entry / Catch Exception] --> Loop
    
    CheckErr -- No --> IsGit{Is folder named '.git'?}
    IsGit -- No --> Loop
    
    IsGit -- Yes --> Found[Increment Repo Count]
    Found --> Color[Set Green Color Output]
    Color --> Exec[Execute: git -C <repo_path> status]
    Exec --> Prune[Disable Recursion inside .git]
    Prune --> Loop
