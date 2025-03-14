# MakePDB

Integrate data and generate a common format.

## Where to get TypeInfo

As we all know, the header files generated by LiteLDev contain type information. But it is not easy to use them directly (we are trying to do so). The current approach is to let the compiler generate PDBs containing type information, and then `makepdb` merges them from them.

## Usage

!!! warning

    Please make sure that the symbol data (generated by askrva) matches the original COFF (bedrock_server.exe).

```
Usage: makepdb [--help] [--version] --program VAR --symbol VAR [--typeinfo VAR] --output VAR

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits
  --program      Path to bedrock_server.exe [required]
  --symbol       Path to symbol data. [required]
  --typeinfo     Path to compiler PDB which contains TPI & IPI (will merged into result PDB).
  -o, --output   Path to output PDB. [required]
```

???+ example

    ```
    ./makepdb --program test.exe --symbol dump.json --output test.pdb
    ```
