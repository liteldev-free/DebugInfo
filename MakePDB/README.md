# MakePDB
Generates a matching PDB file from the symbol data and the original COFF file.

### Usage

> [!NOTE]
> Please make sure that the symbol data (generated by askrva) matches the original COFF (bedrock_server.exe).

```
Usage: makepdb [--help] [--version] --program VAR --symbol-data VAR --output VAR

Optional arguments:
  -h, --help     shows help message and exits 
  -v, --version  prints version information and exits 
  --program      Path to bedrock_server.exe [required]
  --symbol-data  Path to symbol data. [required]
  -o, --output   Path to output PDB. [required]
```

 - Example:
```
./makepdb --program test.exe --symbol-data dump.json --output test.pdb
```
