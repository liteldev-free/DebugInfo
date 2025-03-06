#include "data_format/bound_symbol_list.h"
#include "data_format/raw_text.h"
#include "data_format/typed_symbol_list.h"

#include <argparse/argparse.hpp>

#if DI_USE_NATIVE_SYMBOL_RESOLVER
#include <pl/SymbolProvider.h>
#else
#include "data_format/magic_blob.h"
#endif

using namespace di;
using namespace di::data_format;

[[nodiscard]] auto load_args(int argc, char* argv[]) {
    argparse::ArgumentParser program("askrva");

    struct {
        std::vector<std::string> m_input_paths;
        std::string              m_output_path;
        std::string              m_magic_blob_path;

        std::optional<std::string> m_output_failed_path;
    } args;

    std::string output_format;

    // clang-format off

    program.add_argument("path")
        .help("Path to the symbol list file.")
        .store_into(args.m_input_paths)
        .nargs(argparse::nargs_pattern::at_least_one)
        .required();

    program.add_argument("--magic-blob")
        .help("Path to magic blob (for builtin-symbol-resolver only).")
        .default_value("bedrock_runtime_data")
        .store_into(args.m_magic_blob_path);

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

    auto args = load_args(argc, argv);

    TypedSymbolList symlist;
    BoundSymbolList bound_symbol_list;
    RawText         failed_list;

    for (const auto& input_path : args.m_input_paths) {
        symlist.read(input_path);
    }

    std::println(
        "{} symbols loaded from {} file(s).",
        symlist.count(),
        args.m_input_paths.size()
    );

#if !DI_USE_NATIVE_SYMBOL_RESOLVER
    MagicBlob magic_blob;
    magic_blob.read(args.m_magic_blob_path);

    std::println("{} entries loaded from magicblob.", magic_blob.count());
#endif

    symlist.for_each([&](const TypedSymbol& symbol) {
        auto& sym = symbol.m_name;
#if DI_USE_NATIVE_SYMBOL_RESOLVER
        auto address = pl::symbol_provider::pl_resolve_symbol_silent_n(
            sym.c_str(),
            sym.size()
        );
        // TODO: imagebase...
#else
        auto entry = magic_blob.query(sym);
#endif
        if (entry) {
            bound_symbol_list
                .record(symbol.m_name, entry->rva, symbol.m_type.is_function());
        } else {
            failed_list.record(symbol.m_name);
        }
    });

    bound_symbol_list.write(args.m_output_path);
    if (args.m_output_failed_path) {
        failed_list.write(*args.m_output_failed_path);
    }

    std::println("Everything is OK.");
    return 0;
} catch (const BaseException& e) {
    std::cerr << e;
    return 1;
} catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
}
