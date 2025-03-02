#include "data_format/magic_blob.h"

#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>

using namespace di;
using namespace di::data_format;

auto load_args(int argc, char* argv[]) {
    argparse::ArgumentParser program("blob-extractor");

    struct {
        std::string m_magic_blob_path;
        std::string m_output_path;
    } args;

    // clang-format off

    program.add_argument("magicblob")
        .help("Path to magic blob.")
        .store_into(args.m_magic_blob_path)
        .required();

    program.add_argument("--output", "-o")
        .help("Path to output symlist.")
        .store_into(args.m_output_path)
        .required();

    // clang-format on

    program.parse_args(argc, argv);

    return args;
}

int main(int argc, char* argv[]) try {

    auto args = load_args(argc, argv);

    MagicBlob blob;
    blob.read(args.m_magic_blob_path);

    nlohmann::json data;
    blob.for_each([&data](hash_t hash, const MagicEntry& entry) {
        data[hash] = entry;
    });

    std::ofstream ofs(args.m_output_path);
    if (!ofs) {
        throw std::runtime_error("Failed to open file!");
    }

    ofs << data.dump(4);

    return 0;
} catch (const std::exception& e) {
    std::println("E: {}", e.what());
    return -1;
}
