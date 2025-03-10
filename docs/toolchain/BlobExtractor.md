# BlobExtractor

This tool is used to extract all data from MagicBlob and is generally used for debugging purposes.

## Usage

```
Usage: blob-extractor [--help] [--version] --output VAR magicblob

Positional arguments:
  magicblob      Path to magic blob. [required]

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits
  -o, --output   Path to output symlist. [required]
```

???+ example

    ```
    ./blob-extractor bedrock_runtime_data --output dump.json
    ```
