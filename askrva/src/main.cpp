#include "pl/SymbolProvider.h"

#include <exception>
#include <fstream>
#include <print>
#include <string>

#include <argparse/argparse.hpp>

#include <nlohmann/json.hpp>

int main(int argc, char** argv) try {

    argparse::ArgumentParser program("askrva");

    // clang-format off

    program.add_argument("path")
        .help("Path to the symbol list file.")
        .required();
    
    program.add_argument("--output", "-o")
        .help("Path to output.")
        .required();
    
    program.add_argument("--output-failed", "-of")
        .help("Path to output failed entries.");

    program.add_argument("--output-format")
        .help("Specify output format.")
        .choices("auto", "text", "fakepdb")
        .default_value("auto");

    // clang-format on

    program.parse_args(argc, argv);

    enum OutputFormat { Text, FakePDB } outputFormat;

    std::string outputFormat_s = program.get<std::string>("--output-format");
    std::string inputPath_s    = program.get<std::string>("path");
    std::string outputPath_s   = program.get<std::string>("--output");

    std::optional<std::string> outputFailedPath_s;
    if (program.is_used("--output-failed")) {
        outputFailedPath_s = program.get<std::string>("--output-failed");
    }

    if (outputFormat_s == "text") {
        outputFormat = Text;
    } else if (outputFormat_s == "fakepdb") {
        outputFormat = FakePDB;
    } else {
        if (outputPath_s.ends_with(".json")) {
            outputFormat = FakePDB;
        } else {
            outputFormat = Text;
        }
    }

    std::ifstream input(inputPath_s);
    std::string   symbol;

    std::ofstream output(outputPath_s);

    std::ofstream output_failed;
    if (outputFailedPath_s) {
        output_failed.open(*outputFailedPath_s);
    }

    nlohmann::json json;

    while (std::getline(input, symbol)) {
        if (symbol.empty()) continue;
        if (auto rva = reinterpret_cast<uintptr_t>(
                pl::symbol_provider::pl_resolve_symbol_silent_n(symbol.data(), symbol.size())
            )) {
            if (outputFormat == FakePDB) {
                json[symbol] = std::format("{:#x}", rva);
            } else {
                output << std::format("[{:#x}] {}\n", rva, symbol);
            }
        } else if (output_failed.is_open()) {
            output_failed << symbol << "\n";
        }
    }

    if (outputFormat == FakePDB) {
        output << json.dump(4);
    }

    std::println("Everything is OK.");
    return 0;
} catch (const std::exception& e) {
    std::println("E: {}", e.what());
    return -1;
}
