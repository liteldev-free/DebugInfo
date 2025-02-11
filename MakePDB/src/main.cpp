#include <argparse/argparse.hpp>

#include "binary/COFF.h"
#include "binary/PDB.h"

#include "symbol_data.h"

using namespace makepdb;

[[nodiscard]] auto load_args(int argc, char* argv[]) {

    argparse::ArgumentParser program("makepdb", "1.1.0");

    struct {
        std::string server_program_path;
        std::string symbol_data_path;
        std::string typeinfo_pdb_path;
        std::string output_path;
    } args;

    program.add_argument("--program")
        .help("Path to bedrock_server.exe")
        .store_into(args.server_program_path)
        .required();

    program.add_argument("--symbol")
        .help("Path to symbol data.")
        .store_into(args.symbol_data_path)
        .required();

    program.add_argument("--output", "-o")
        .help("Path to output PDB.")
        .store_into(args.output_path)
        .required();

    program.parse_args(argc, argv);

    return args;
}

int main(int argc, char* argv[]) try {

    auto args = load_args(argc, argv);

    binary::COFF server_program(args.server_program_path);
    SymbolData   symbol_data(args.symbol_data_path);

    binary::PDB pdb(std::move(server_program), std::move(symbol_data));

    pdb.write(args.output_path);

    return 0;
} catch (const std::exception& e) {
    std::println("E: {}", e.what());
    return -1;
}
