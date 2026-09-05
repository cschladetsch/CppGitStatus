#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <print>
#include <vector>
#include <string>
#include <system_error>
#include <rang.hpp>

namespace fs = std::filesystem;

// RAII helper to ensure color styles are always reset properly
struct ScopedColor {
    rang::fg color;
    ScopedColor(rang::fg c) : color(c) { std::cout << color; }
    ~ScopedColor() { std::cout << rang::style::reset; }
};

// Safe path-to-string conversion for Windows multi-byte code pages
std::string safe_path_string(const fs::path& p) {
    try {
        return p.string();
    } catch (...) {
        try {
            return p.generic_string();
        } catch (...) {
            return "";
        }
    }
}

struct Options {
    fs::path parent_path;
    bool short_summary = false;
    int target_index = -1;
};

struct RepoStatusInfo {
    std::string branch_info = "unknown";
    int modified_count = 0;
    int untracked_count = 0;
    int staged_count = 0;
    bool is_clean = true;
};

bool parse_arguments(int argc, char* argv[], Options& opts) {
    if (argc < 2) {
        ScopedColor col(rang::fg::red);
        std::println(stderr, "Error: No parent folder specified.");
        std::println(stderr, "Usage: {} <parent-folder-path> [-s | repo-index]", argv[0]);
        return false;
    }

    opts.parent_path = fs::absolute(argv[1]);
    std::error_code ec;
    if (!fs::exists(opts.parent_path, ec) || !fs::is_directory(opts.parent_path, ec)) {
        ScopedColor col(rang::fg::red);
        std::println(stderr, "Error: '{}' is not a valid directory.", argv[1]);
        return false;
    }

    if (argc >= 3) {
        std::string arg2 = argv[2];
        if (arg2 == "-s") {
            opts.short_summary = true;
        } else {
            try {
                opts.target_index = std::stoi(arg2);
            } catch (...) {
                ScopedColor col(rang::fg::red);
                std::println(stderr, "Error: Invalid argument or repository index '{}'.", arg2);
                return false;
            }
        }
    }
    return true;
}

bool should_skip_directory(const std::string& dirname) {
    return dirname == "build" || dirname == "build_deps" || dirname == "vcpkg" || 
           dirname == "node_modules" || dirname == ".vs" || dirname == ".gradle";
}

// Full status output for standard single-repo inspection
void run_git_status_full(const std::string& repo_path) {
    std::string cmd = "git -C \"" + repo_path + "\" status -sb";
    std::system(cmd.c_str());
}

// Intelligent status parser for short summary mode
RepoStatusInfo query_repo_status(const std::string& repo_path) {
    RepoStatusInfo info;
    std::string cmd = "git -C \"" + repo_path + "\" status --porcelain=v1 -b";

#if defined(_WIN32) || defined(_WIN64)
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe) return info;

    char buffer[256];
    bool first_line = true;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (!line.empty() && line.back() == '\n') line.pop_back();
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (first_line) {
            first_line = false;
            if (line.rfind("##", 0) == 0) {
                info.branch_info = line.substr(3); // Strip "## " prefix
            } else {
                info.branch_info = line;
            }
        } else {
            info.is_clean = false;
            if (line.rfind("??", 0) == 0) {
                info.untracked_count++;
            } else if (line.length() >= 2) {
                char x = line[0];
                char y = line[1];
                if (x != ' ' && x != '?') {
                    info.staged_count++;
                }
                if (y != ' ' && y != '?') {
                    info.modified_count++;
                }
            }
        }
    }

#if defined(_WIN32) || defined(_WIN64)
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return info;
}

int main(int argc, char* argv[]) {
    try {
        Options opts;
        if (!parse_arguments(argc, argv, opts)) {
            return 1;
        }

        std::vector<fs::path> found_repos;
        int repo_count = 0;

        std::error_code ec;
        auto it = fs::recursive_directory_iterator(
            opts.parent_path, 
            fs::directory_options::skip_permission_denied, 
            ec
        );

        if (ec) {
            ScopedColor col(rang::fg::red);
            std::println(stderr, "Error opening directory iterator: {}", ec.message());
            return 1;
        }

        auto end = fs::recursive_directory_iterator();
        while (it != end) {
            fs::path current_path;
            bool is_dir = false;
            try {
                current_path = it->path();
                is_dir = it->is_directory(ec);
            } catch (...) {
                it.increment(ec);
                if (ec) break;
                continue;
            }

            if (!ec && is_dir) {
                std::string dirname = safe_path_string(current_path.filename());

                if (should_skip_directory(dirname)) {
                    it.disable_recursion_pending();
                }
                else if (dirname == ".git") {
                    fs::path repo_path = current_path.parent_path();
                    repo_count++;
                    found_repos.push_back(repo_path);

                    if (opts.target_index > 0 && repo_count == opts.target_index) {
                        break;
                    }

                    // Print immediately in standard mode (non-summary, non-indexed)
                    if (!opts.short_summary && opts.target_index < 0) {
                        {
                            ScopedColor col(rang::fg::green);
                            std::println("[{}] {}", repo_count, safe_path_string(repo_path));
                        }
                        run_git_status_full(safe_path_string(repo_path));
                        std::println("");
                    }

                    it.disable_recursion_pending();
                }
            }

            it.increment(ec);
            if (ec) {
                ec.clear();
                it.increment(ec);
                if (ec) break;
            }
        }

        // Summary, Index, or Completion reporting
        if (opts.short_summary) {
            if (found_repos.empty()) {
                ScopedColor col(rang::fg::yellow);
                std::println(stderr, "No git repositories found under '{}'.", safe_path_string(opts.parent_path));
            } else {
                for (size_t i = 0; i < found_repos.size(); ++i) {
                    std::string repo_str = safe_path_string(found_repos[i]);
                    if (repo_str.empty()) continue;

                    RepoStatusInfo status = query_repo_status(repo_str);

                    {
                        ScopedColor col(rang::fg::cyan);
                        std::print("[{}] {:<30} ", i + 1, safe_path_string(found_repos[i].filename()));
                    }
                    
                    std::print("-> {}", status.branch_info);

                    if (status.is_clean) {
                        ScopedColor col(rang::fg::green);
                        std::println(" (clean)");
                    } else {
                        ScopedColor col(rang::fg::yellow);
                        std::print(" [");
                        bool comma = false;
                        if (status.staged_count > 0) {
                            std::print("{} staged", status.staged_count);
                            comma = true;
                        }
                        if (status.modified_count > 0) {
                            if (comma) std::print(", ");
                            std::print("{} modified", status.modified_count);
                            comma = true;
                        }
                        if (status.untracked_count > 0) {
                            if (comma) std::print(", ");
                            std::print("{} untracked", status.untracked_count);
                        }
                        std::println("]");
                    }
                }
            }
        } else if (opts.target_index > 0) {
            if (opts.target_index <= static_cast<int>(found_repos.size())) {
                std::println("{}", safe_path_string(found_repos[opts.target_index - 1]));
            } else {
                ScopedColor col(rang::fg::red);
                std::println(stderr, "Error: Repository index {} out of range (found {} repos).", opts.target_index, found_repos.size());
                return 1;
            }
        } else if (opts.target_index < 0 && repo_count == 0) {
            ScopedColor col(rang::fg::yellow);
            std::println(stderr, "No git repositories found under '{}'.", safe_path_string(opts.parent_path));
        } else if (opts.target_index < 0) {
            ScopedColor col(rang::fg::cyan);
            std::println(stderr, "Scan complete. Found {} repository/repositories.", repo_count);
        }

    } catch (const std::exception& e) {
        ScopedColor col(rang::fg::red);
        std::println(stderr, "Fatal Exception: {}", e.what());
        return 1;
    } catch (...) {
        ScopedColor col(rang::fg::red);
        std::println(stderr, "Fatal Unknown Exception occurred.");
        return 1;
    }

    return 0;
}
