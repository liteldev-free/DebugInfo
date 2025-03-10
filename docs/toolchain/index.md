# Getting started

All the tools you need are here!

## Introduction

- `DeThunk` - Header file preprocessing.
- `DumpSYM` - Extract symbols from header files.
- `ExtractSYM` - Extracting symbols from elsewhere.
- `AskRVA` - Batch query the RVA of symbols.
- `MakePDB` - Generate debugging information in a common format.
- `BlobExtractor` - Explore MagicBlob.

## Build

!!! tip

    You can get prebuilt versions from [Releases](https://github.com/liteldev-free/DebugInfo/releases) or [GitHub Actions](https://github.com/liteldev-free/DebugInfo/actions).

Install build dependencies

```
pacman -S llvm clang git xmake
```

Clone the repository

```
git clone https://github.com/liteldev-free/DebugInfo.git
```

Configure and build

```
xmake
```

??? danger "Advanced: Use the native symbol resolver"

    Switching to the native symbol resolver is not recommended unless you have a enough reason to do so. <br>
    **This will rely on PreLoader, which is closed source software of unknown security and is only available on Windows.**

    ```
    xmake f --symbol-resolver=native
    ```

Will automatically install build dependencies and build all targets.
