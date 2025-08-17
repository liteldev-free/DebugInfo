#include "data_format/magic_blob.h"

#include <argparse/argparse.hpp>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

using namespace di;
using namespace di::data_format;
using namespace di::io;

auto load_args(int argc, char* argv[]) {
    argparse::ArgumentParser program("blob-extractor");

    struct {
        std::string m_magic_blob_path;
        fs::path    m_output_path;

        std::string m_format_version;
    } args;

    // clang-format off

    program.add_argument("magicblob")
        .help("Path to magic blob.")
        .store_into(args.m_magic_blob_path)
        .required();

    program.add_argument("--output", "-o")
        .help("Path to output symlist.")
        .required();

    std::apply([&](auto&&... xs) {
        program.add_argument("--preloader-version")
            .help("Choose a compatible PreLoader version. (for builtin-symbol-resolver only).")
            .choices(xs...)
            .store_into(args.m_format_version)
            .required();
    }, magic_enum::enum_names<MagicBlob::FormatVersion>());

    // clang-format on

    program.parse_args(argc, argv);

    args.m_output_path = program.get("--output");

    return args;
}

int main(int argc, char* argv[]) try {

    auto args = load_args(argc, argv);

    auto format_version =
        magic_enum::enum_cast<MagicBlob::FormatVersion>(args.m_format_version);
    assert(format_version.has_value());

    auto magic_blob = MagicBlob::create(*format_version);
    if (!magic_blob) {
        std::println(
            "Format version: {} is not yet implemented.",
            args.m_format_version
        );
    }

    magic_blob->read(args.m_magic_blob_path);

    nlohmann::json data;
    magic_blob->for_each([&data](hash_t hash, MagicBlob::shared_entry_t entry) {
        entry->to_json(data.emplace_back());
    });

    std::ofstream ofs(args.m_output_path);
    if (!ofs) throw UnableToOpenException(args.m_output_path);

    ofs << data.dump(4);

    return 0;
} catch (const BaseException& e) {
    std::cerr << e;
    return 1;
} catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
}
