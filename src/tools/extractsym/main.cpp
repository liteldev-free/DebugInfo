#include <llvm/DebugInfo/CodeView/CVRecord.h>
#include <llvm/DebugInfo/CodeView/SymbolDeserializer.h>
#include <llvm/DebugInfo/PDB/IPDBSession.h>
#include <llvm/DebugInfo/PDB/Native/NativeSession.h>
#include <llvm/DebugInfo/PDB/Native/PDBFile.h>
#include <llvm/DebugInfo/PDB/Native/PublicsStream.h>
#include <llvm/DebugInfo/PDB/Native/SymbolStream.h>
#include <llvm/DebugInfo/PDB/PDB.h>

#include <argparse/argparse.hpp>

using namespace llvm;
using namespace llvm::pdb;
using namespace llvm::codeview;

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

    std::unique_ptr<IPDBSession> pdb_session;
    if (llvm::pdb::loadDataForPDB(
            PDB_ReaderType::Native,
            args.m_program_database_path,
            pdb_session
        )) {
        throw std::runtime_error("Failed to load PDB.");
    }

    auto  native_session = static_cast<NativeSession*>(pdb_session.get());
    auto& pdb_file       = native_session->getPDBFile();

    auto publics_stream = pdb_file.getPDBPublicsStream();
    if (!publics_stream) {
        throw std::runtime_error("Failed to get public stream from PDB.");
    }

    auto publics_symbol_stream = pdb_file.getPDBSymbolStream();
    if (!publics_symbol_stream) {
        throw std::runtime_error("Failed to get symbol stream from PDB.");
    }

    std::ofstream ofs(args.m_output_path);
    if (!ofs) {
        throw std::runtime_error("Failed to open output file.");
    }

    auto publics_symbols =
        publics_symbol_stream->getSymbolArray().getUnderlyingStream();
    for (auto offset : publics_stream->getPublicsTable()) {
        auto cv_symbol = readSymbolFromStream(publics_symbols, offset);
        auto public_sym32 =
            SymbolDeserializer::deserializeAs<PublicSym32>(cv_symbol.get());
        if (!public_sym32) {
            throw std::runtime_error("Unsupported symbol type.");
        }

        ofs
            << ((public_sym32->Flags & PublicSymFlags::Function)
                        != PublicSymFlags::None
                    ? "Function, "
                    : "Var, ")
            << public_sym32->Name.str() << "\n";
    }

    return 0;
} catch (const std::exception& e) {
    std::println("E: {}", e.what());
    return -1;
}
