#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <print>
#include <rang.hpp>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cout << rang::fg::red;
            std::println("Error: No parent folder specified.");
            std::cout << rang::style::reset;
            std::println("Usage: {} <parent-folder-path>", argv[0]);
            return 1;
        }

        fs::path parent_path(argv[1]);

        if (!fs::exists(parent_path) || !fs::is_directory(parent_path)) {
            std::cout << rang::fg::red;
            std::println("Error: '{}' is not a valid directory.", argv[1]);
            std::cout << rang::style::reset;
            return 1;
        }

        std::cout << rang::fg::cyan;
        std::println("Scanning '{}' for git repositories...\n", parent_path.string());
        std::cout << rang::style::reset;

        int repo_count = 0;

        // Using error_code overload to prevent iterator crashes on restricted subpaths
        std::error_code ec;
        auto it = fs::recursive_directory_iterator(
            parent_path, 
            fs::directory_options::skip_permission_denied, 
            ec
        );

        if (ec) {
            std::cout << rang::fg::red;
            std::println("Error opening directory iterator: {}", ec.message());
            std::cout << rang::style::reset;
            return 1;
        }

        auto end = fs::recursive_directory_iterator();
        while (it != end) {
            // Guard individual entry checks
            fs::path current_path;
            bool is_dir = false;
            try {
                current_path = it->path();
                is_dir = it->is_directory(ec);
            } catch (...) {
                // Skip problematic entries silently
                it.increment(ec);
                if (ec) break;
                continue;
            }

            if (!ec && is_dir && current_path.filename() == ".git") {
                fs::path repo_path = current_path.parent_path();
                repo_count++;

                std::cout << rang::fg::green;
                std::println("========================================");
                std::println("Repository [{}] found at: {}", repo_count, repo_path.string());
                std::println("========================================");
                std::cout << rang::style::reset;

                std::string cmd = "git -C \"" + repo_path.string() + "\" status";
                std::system(cmd.c_str());
                std::println(""); // Blank line for spacing

                it.disable_recursion_pending();
            }

            it.increment(ec);
            if (ec) {
                // Clear error and attempt to move forward if possible
                ec.clear();
                it.increment(ec);
                if (ec) break;
            }
        }

        if (repo_count == 0) {
            std::cout << rang::fg::yellow;
            std::println("No git repositories found under '{}'.", argv[1]);
            std::cout << rang::style::reset;
        } else {
            std::cout << rang::fg::cyan;
            std::println("Scan complete. Found {} repository/repositories.", repo_count);
            std::cout << rang::style::reset;
        }

    } catch (const std::exception& e) {
        std::cout << rang::fg::red;
        std::println("Fatal Exception: {}", e.what());
        std::cout << rang::style::reset;
        return 1;
    } catch (...) {
        std::cout << rang::fg::red;
        std::println("Fatal Unknown Exception occurred.");
        std::cout << rang::style::reset;
        return 1;
    }

    return 0;
}
