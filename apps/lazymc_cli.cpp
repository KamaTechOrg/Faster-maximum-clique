// lazymc_cli.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "lazymc/lazymc.hpp"

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options] <graph_file>\n"
              << "Options:\n"
              << "  -h, --help     Show this help message\n"
              << "  -v, --verbose  Verbose output\n"
              << "  -t, --timeout  Timeout in seconds (default: no timeout)\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string filename;
    bool verbose = false;
    double timeout = 0.0;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-t" || arg == "--timeout") {
            if (i + 1 < argc) {
                timeout = std::stod(argv[++i]);
            }
        } else {
            filename = arg;
        }
    }

    if (filename.empty()) {
        std::cerr << "Error: No input file specified\n";
        print_usage(argv[0]);
        return 1;
    }

    // TODO: Load graph from file
    // כרגע נשתמש בגרף לדוגמה
    std::vector<std::vector<int>> adj = {
        {1, 2, 3},    // vertex 0
        {0, 2, 3},    // vertex 1
        {0, 1, 3},    // vertex 2
        {0, 1, 2}     // vertex 3
    };

    if (verbose) {
        std::cout << "Graph loaded: " << adj.size() << " vertices\n";
    }

    // Run algorithm
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<int> max_clique = lazymc::find_maximum_clique(adj);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Output results
    std::cout << "Maximum clique size: " << max_clique.size() << "\n";
    std::cout << "Vertices: ";
    for (int v : max_clique) {
        std::cout << v << " ";
    }
    std::cout << "\n";

    if (verbose) {
        std::cout << "Time: " << duration.count() << " ms\n";
    }

    return 0;
}
