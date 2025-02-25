# AskRVA

This module uses LiteLDev's official PreLoader to load bedrock_runtime_data, calls `pl::symbol_provider::pl_resolve_symbol` to obtain the RVA corresponding to the symbol, and then builds the symbol table.

> [!WARNING]
> AskRVA must load PreLoader, and the security of PreLoader is unknown (see below for the reason). It is recommended to execute it in a restricted environment. The author does not bear any consequences.

### PreLoader

[PreLoader](https://github.com/LiteLDev/PreLoader) is a key component of LeviLamina, which is responsible for parsing bedrock_runtime_data, hooking, calling, loading LeviLamina ontology, etc.  
**PreLoader is currently proprietary software.** After v1.10.0, due to the adoption of bedrock_runtime_data (also
known as magicblob), PreLoader no longer handles PDB. The source code in the current repository is old, as of v1.9.2.

### Build

- Since there is no Linux/MacOS build for PreLoader, you have to build AskRVA under Windows.
- Just execute `xmake` to complete the build.

### Usage

- --output-format can be `auto` / `txt` / `fakepdb` / `makepdb`

```
Usage: askrva [--help] [--version] --output VAR [--output-failed VAR] [--output-format VAR] path

Positional arguments:
  path                  Path to the symbol list file. [required]

Optional arguments:
  -h, --help            shows help message and exits
  -v, --version         prints version information and exits
  -o, --output          Path to output. [required]
  -of, --output-failed  Path to output failed entries.
  --output-format       Specify output format. [nargs=0..1] [default: "auto"]
```

- Example:

```
./askrva.exe __cpp_main.cpp.cpp.symbols --output output.json --output-failed failed.txt
```
