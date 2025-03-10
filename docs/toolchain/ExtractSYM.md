# ExtractSYM

This tool is used to extract symbols from PDB.

## Usage

```
Usage: extractsym [--help] [--version] --output VAR pdb

Positional arguments:
  pdb            Path to program database. [required]

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits
  -o, --output   Path to output symlist. [required]
```

???+ example

    ```
    ./extractsym bedrock_server.pdb --output symbols.txt
    ```
