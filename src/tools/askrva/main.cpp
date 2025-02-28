#include "data_format/bound_symbol_list.h"
#include "data_format/raw_text.h"
#include "data_format/typed_symbol_list.h"

#include <argparse/argparse.hpp>

#if DI_USE_NATIVE_SYMBOL_RESOLVER
#include <pl/SymbolProvider.h>
#endif

using namespace di;

[[nodiscard]] auto load_args(int argc, char* argv[]) {
    argparse::ArgumentParser program("askrva");

    struct {
        std::vector<std::string> m_input_paths;
        std::string              m_output_path;

        std::optional<std::string> m_output_failed_path;
    } args;

    std::string output_format;

    // clang-format off

    program.add_argument("path")
        .help("Path to the symbol list file.")
        .store_into(args.m_input_paths)
        .nargs(argparse::nargs_pattern::at_least_one)
        .required();
    
    program.add_argument("--output", "-o")
        .help("Path to output.")
        .store_into(args.m_output_path)
        .required();
    
    program.add_argument("--output-failed", "-of")
        .help("Path to output failed entries.");

    program.add_argument("--output-format")
        .help("Specify output format.")
        .choices("auto", "text", "makepdb")
        .default_value("auto")
        .store_into(output_format);

    // clang-format on

    program.parse_args(argc, argv);

    if (program.is_used("--output-failed")) {
        args.m_output_failed_path = program.get<std::string>("--output-failed");
    }

    return args;
}

int main(int argc, char* argv[]) try {

    auto args    = load_args(argc, argv);
    auto symlist = data_format::TypedSymbolList();

    data_format::BoundSymbolList bound_symbol_list;
    data_format::RawText         raw_text;

    symlist.for_each([&](const TypedSymbol& symbol) {
        auto& sym = symbol.m_name;
#if DI_USE_NATIVE_SYMBOL_RESOLVER
        auto rva = pl::symbol_provider::pl_resolve_symbol_silent_n(
            sym.c_str(),
            sym.size()
        );
#else
        auto rva = (void*)nullptr; // TODO
#endif
        if (rva) {
            bound_symbol_list.record(
                symbol.m_name,
                reinterpret_cast<uint64_t>(rva),
                symbol.m_type.is_function()
            );
        } else {
            raw_text.record(symbol.m_name);
        }
    });

    bound_symbol_list.write(args.m_output_path);
    if (args.m_output_failed_path) {
        raw_text.write(*args.m_output_failed_path);
    }

    std::println("Everything is OK.");
    return 0;
} catch (const std::exception& e) {
    std::println("E: {}", e.what());
    return -1;
}
