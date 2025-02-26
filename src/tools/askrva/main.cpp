#include "util/string.h"

#include "data_format/typed_symbol_list.h"

#include <argparse/argparse.hpp>

#if DI_USE_NATIVE_SYMBOL_RESOLVER
#include <pl/SymbolProvider.h>
#endif

constexpr auto VERSION = "1.0.0";

enum class OutputFormat { Text, MakePDB };

using namespace di;

[[nodiscard]] auto load_args(int argc, char* argv[]) {
    argparse::ArgumentParser program("askrva", VERSION);

    struct {
        OutputFormat             m_output_format;
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

    args.m_output_format = [&output_format, &args]() -> OutputFormat {
        using namespace util::string;

        switch (H(output_format)) {
        case H("text"):
            return OutputFormat::Text;
        case H("makepdb"):
            return OutputFormat::MakePDB;
        case H("auto"):
        default: {
            if (args.m_output_path.ends_with(".json")) {
                return OutputFormat::MakePDB;
            } else {
                return OutputFormat::Text;
            }
        }
        }
    }();

    if (program.is_used("--output-failed")) {
        args.m_output_failed_path = program.get<std::string>("--output-failed");
    }

    return args;
}

int main(int argc, char* argv[]) try {

    auto args    = load_args(argc, argv);
    auto symlist = data_format::TypedSymbolList(args.m_input_paths);

    auto output_file = (args.m_output_format);

    symlist.for_each([&output_file](const TypedSymbol& symbol) {
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
            output_file->record(
                symbol.m_name,
                reinterpret_cast<uint64_t>(rva),
                symbol.m_type.is_function()
            );
        } else {
            output_file->record_failure(symbol.m_name);
        }
    });

    output_file->save(args.m_output_path);
    if (args.m_output_failed_path) {
        output_file->save_failure(*args.m_output_failed_path);
    }

    std::println("Everything is OK.");
    return 0;
} catch (const std::exception& e) {
    std::println("E: {}", e.what());
    return -1;
}
