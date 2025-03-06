#include <argparse/argparse.hpp>

#include <llvm/DebugInfo/CodeView/SymbolRecord.h>

#include "data_format/typed_symbol_list.h"
#include "object_file/pdb.h"

using namespace di;
using namespace di::object_file;
using namespace di::data_format;

auto load_args(int argc, char* argv[]) {
    argparse::ArgumentParser program("extractpdb");

    struct {
        std::string m_program_database_path;
        std::string m_output_path;
    } args;

    // clang-format off

    program.add_argument("--output", "-o")
        .help("Path to output symlist.")
        .store_into(args.m_output_path)
        .required();

    program.add_argument("pdb")
        .help("Path to program database.")
        .store_into(args.m_program_database_path)
        .required();

    // clang-format on

    program.parse_args(argc, argv);

    return args;
}

int main(int argc, char* argv[]) try {

    auto args = load_args(argc, argv);

    PDB pdb;
    pdb.read(args.m_program_database_path);

    TypedSymbolList symbol_list;

    pdb.for_each<PDB::Public>([&symbol_list](const codeview::PublicSym32& symbol
                              ) {
        using codeview::PublicSymFlags;

        auto is_fun =
            (symbol.Flags & PublicSymFlags::Function) != PublicSymFlags::None;
        symbol_list.record(
            symbol.Name.str(),
            is_fun ? DeclType::Function : DeclType::Var
        );
    });

    symbol_list.write(args.m_output_path);

    return 0;
} catch (const BaseException& e) {
    std::cerr << e;
    return 1;
} catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
}