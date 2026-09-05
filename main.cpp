#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <print>
#include <vector>
#include <string>
#include <rang.hpp>

namespace fs = std::filesystem;

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

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cout << rang::fg::red;
            std::println(stderr, "Error: No parent folder specified.");
            std::cout << rang::style::reset;
            std::println(stderr, "Usage: {} <parent-folder-path> [-s | repo-index]", argv[0]);
            return 1;
        }

        fs::path parent_path = fs::absolute(argv[1]);

        if (!fs::exists(parent_path) || !fs::is_directory(parent_path)) {
            std::cout << rang::fg::red;
            std::println(stderr, "Error: '{}' is not a valid directory.", argv[1]);
            std::cout << rang::style::reset;
            return 1;
        }

        bool short_summary = false;
        int target_index = -1;

        if (argc >= 3) {
            std::string arg2 = argv[2];
            if (arg2 == "-s") {
                short_summary = true;
            } else {
                try {
                    target_index = std::stoi(arg2);
                } catch (...) {
                    std::cout << rang::fg::red;
                    std::println(stderr, "Error: Invalid argument or repository index '{}'.", arg2);
                    std::cout << rang::style::reset;
                    return 1;
                }
            }
        }

        int repo_count = 0;
        std::vector<fs::path> found_repos;

        std::error_code ec;
        auto it = fs::recursive_directory_iterator(
            parent_path, 
            fs::directory_options::skip_permission_denied, 
            ec
        );

        if (ec) {
            std::cout << rang::fg::red;
            std::println(stderr, "Error opening directory iterator: {}", ec.message());
            std::cout << rang::style::reset;
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

                if (dirname == "build" || dirname == "build_deps" || dirname == "vcpkg" || 
                    dirname == "node_modules" || dirname == ".vs" || dirname == ".gradle") {
                    it.disable_recursion_pending();
                }
                else if (dirname == ".git") {
                    fs::path repo_path = current_path.parent_path();
                    repo_count++;
                    found_repos.push_back(repo_path);

                    if (target_index > 0 && repo_count == target_index) {
                        break;
                    }

                    if (!short_summary && target_index < 0) {
                        std::cout << rang::fg::green;
                        std::println("[{}] {}", repo_count, safe_path_string(repo_path));
                        std::cout << rang::style::reset;

                        std::string cmd = "git -C \"" + safe_path_string(repo_path) + "\" status -sb";
                        std::system(cmd.c_str());
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

        if (short_summary) {
            if (found_repos.empty()) {
                std::cout << rang::fg::yellow;
                std::println(stderr, "No git repositories found under '{}'.", safe_path_string(parent_path));
                std::cout << rang::style::reset;
            } else {
                for (size_t i = 0; i < found_repos.size(); ++i) {
                    std::string repo_str = safe_path_string(found_repos[i]);
                    if (repo_str.empty()) continue;

                    std::cout << rang::fg::cyan;
                    std::print("[{}] {:<30} ", i + 1, safe_path_string(found_repos[i].filename()));
                    std::cout << rang::style::reset << std::flush;

                    std::string cmd = "git -C \"" + repo_str + "\" status -sb";
                    std::system(cmd.c_str());
                }
            }
        } else if (target_index > 0) {
            if (target_index <= static_cast<int>(found_repos.size())) {
                std::println("{}", safe_path_string(found_repos[target_index - 1]));
            } else {
                std::cout << rang::fg::red;
                std::println(stderr, "Error: Repository index {} out of range (found {} repos).", target_index, found_repos.size());
                std::cout << rang::style::reset;
                return 1;
            }
        } else if (target_index < 0 && repo_count == 0) {
            std::cout << rang::fg::yellow;
            std::println(stderr, "No git repositories found under '{}'.", safe_path_string(parent_path));
            std::cout << rang::style::reset;
        } else if (target_index < 0) {
            std::cout << rang::fg::cyan;
            std::println(stderr, "Scan complete. Found {} repository/repositories.", repo_count);
            std::cout << rang::style::reset;
        }

    } catch (const std::exception& e) {
        std::cout << rang::fg::red;
        std::println(stderr, "Fatal Exception: {}", e.what());
        std::cout << rang::style::reset;
        return 1;
    } catch (...) {
        std::cout << rang::fg::red;
        std::println(stderr, "Fatal Unknown Exception occurred.");
        std::cout << rang::style::reset;
        return 1;
    }

    return 0;
}
