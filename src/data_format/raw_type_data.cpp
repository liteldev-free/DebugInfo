#include "data_format/raw_type_data.h"

#include <llvm/DebugInfo/CodeView/TypeStreamMerger.h>
#include <llvm/DebugInfo/PDB/IPDBSession.h>
#include <llvm/DebugInfo/PDB/Native/NativeSession.h>
#include <llvm/DebugInfo/PDB/Native/PDBFile.h>
#include <llvm/DebugInfo/PDB/Native/TpiStream.h>
#include <llvm/DebugInfo/PDB/PDB.h>
#include <llvm/DebugInfo/PDB/PDBTypes.h>

using namespace llvm::pdb;

namespace di::data_format {

void RawTypeData::read(const fs::path& path) {
    std::unique_ptr<IPDBSession> pdb_session;
    if (llvm::pdb::loadDataForPDB(
            PDB_ReaderType::Native,
            path.string(),
            pdb_session
        )) {
        throw std::runtime_error("Failed to load PDB.");
    }

    auto  native_session = static_cast<NativeSession*>(pdb_session.get());
    auto& pdb_file       = native_session->getPDBFile();

    SmallVector<codeview::TypeIndex, 128> type_map;
    SmallVector<codeview::TypeIndex, 128> id_map;

    if (auto tpi_stream = pdb_file.getPDBTpiStream()) {
        if (codeview::mergeTypeRecords(
                m_storaged_TPI,
                type_map,
                (*tpi_stream).typeArray()
            )) {
            throw std::runtime_error("Failed to merge type record.");
        }
    } else {
        throw std::runtime_error("TPI is not valid.");
    }

    if (auto ipi_stream = pdb_file.getPDBIpiStream()) {
        if (codeview::mergeIdRecords(
                m_storaged_IPI,
                type_map,
                id_map,
                (*ipi_stream).typeArray()
            )) {
            throw std::runtime_error("Failed to merge id record.");
        }
    } else {
        throw std::runtime_error("IPI is not valid.");
    }
}

} // namespace di::data_format
