#include <argparse/argparse.hpp>

#include "object_file/coff.h"
#include "object_file/pdb.h"

#include "data_format/bound_symbol_list.h"
#include "data_format/raw_type_data.h"

using namespace di;

[[nodiscard]] auto load_args(int argc, char* argv[]) {

    argparse::ArgumentParser program("makepdb");

    struct {
        std::string server_program_path;
        std::string symbol_data_path;
        std::string output_path;

        std::optional<std::string> typeinfo_pdb_path;
    } args;

    program.add_argument("--program")
        .help("Path to bedrock_server.exe")
        .store_into(args.server_program_path)
        .required();

    program.add_argument("--symbol")
        .help("Path to symbol data.")
        .store_into(args.symbol_data_path)
        .required();

    program.add_argument("--typeinfo")
        .help("Path to compiler PDB which contains TPI & IPI (will merged into "
              "result PDB).");

    program.add_argument("--output", "-o")
        .help("Path to output PDB.")
        .store_into(args.output_path)
        .required();

    program.parse_args(argc, argv);

    if (program.is_used("--typeinfo")) {
        args.typeinfo_pdb_path = program.get<std::string>("--typeinfo");
    }

    return args;
}

int main(int argc, char* argv[]) try {

    auto args = load_args(argc, argv);

    auto server_program =
        std::make_unique<object_file::COFF>(args.server_program_path);

    auto symbol_data = std::make_unique<data_format::BoundSymbolList>();
    symbol_data->read(args.symbol_data_path);

    std::unique_ptr<data_format::RawTypeData> raw_type_data;
    if (args.typeinfo_pdb_path) {
        raw_type_data = std::make_unique<data_format::RawTypeData>();
        raw_type_data->read(*args.typeinfo_pdb_path);
    }

    object_file::PDB pdb;
    pdb.set_coff_object(std::move(server_program));
    pdb.set_symbol_data(std::move(symbol_data));
    pdb.set_raw_type_data(std::move(raw_type_data));

    pdb.write(args.output_path);

    return 0;
} catch (const std::exception& e) {
    std::println("E: {}", e.what());
    return -1;
}
