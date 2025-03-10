# AskRVA

This tool is used to parse bedrock_runtime_data (MagicBlob), accepts the results of DumpSYM/ExtractSYM as input and outputs a symbol table in JSON format.

## MagicBlob parsing backend

The libdi provides two parser backends: `builtin` and `native`. By default, the built-in is used.

- `builtin` is an open source MagicBlob format implementation that supports cross-platform.
- `native` uses LiteLDev's [PreLoader](https://github.com/LiteLDev/Preloader), which is closed source and only supports Windows platform.

The choice of which backend to use is controlled by build options, see the Build section.

!!! warning

      If you do need to use the native backend, make sure you do it in a restricted environment.

## Usage

```
Usage: askrva [--help] [--version] [--magic-blob VAR] --output VAR [--output-failed VAR] path...

Positional arguments:
  path                  Path to the symbol list file. [nargs: 1 or more] [required]

Optional arguments:
  -h, --help            shows help message and exits
  -v, --version         prints version information and exits
  --magic-blob          Path to magic blob (for builtin-symbol-resolver only). [nargs=0..1] [default: "bedrock_runtime_data"]
  -o, --output          Path to output. [required]
  -of, --output-failed  Path to output failed entries.
```

!!! tip

    If the native symbol resolver is used, MagicBlob must be the "bedrock_runtime_data" file in the current directory. Custom read paths are not supported.

???+ example

    ```
    ./askrva symlist.txt --output succeed.json --output-failed failed.txt
    ```
