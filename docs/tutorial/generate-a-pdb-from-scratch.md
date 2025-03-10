# Generate your first PDB

!!! tip

    All operations in this article are performed under Linux.<br>

:octicons-git-commit-24: DebugInfo: [ccac962](https://github.com/liteldev-free/DebugInfo/commit/ccac9623d90cf22862259bbde4af7c2df05da19f)  
:octicons-git-commit-24: Header: [642a16f](https://github.com/liteldev-free/Header/commit/642a16f8a6db364b1ff3d0a8e917c77de42d588b)  
:material-package-variant-closed-check: Bedrock Dedicated Server: [1.21.60.10](https://www.minecraft.net/bedrockdedicatedserver/bin-linux/bedrock-server-1.21.60.10.zip)

Today we will generate a complete PDB, including symbol tables and type information.  
Before we start, I assume that you have followed the build instructions, or downloaded a pre-built, all in all you should have a complete _DebugInfo_ toolchain.

## Install build dependencies

### Setup msvc-wine

Since everything is done under Linux, and the header depends on MSSTL, you must install [msvc-wine](https://github.com/mstorsjo/msvc-wine) (not just clang or clang-cl). If you are working under Windows, you should only need to install Visual Studio.

```
yay -S msvc-wine
```

### Setup clang & xmake

Because the header file project uses the xmake build system.

```
pacman -S clang xmake
```

## Get the header files

We use `liteldev-free/Header` instead of the header branch of `liteldev/LeviLamina` because we made some patches on the former.  
`liteldev-free/Header` is synced from upstream for each version from time to time.

```
git clone https://github.com/liteldev-free/Header.git
```

Switch to the desired branch. The `_p` suffix means "patched".

```
git checkout -b mc/r21_u6_p
```

## Prepare other things

<div class="annotate" markdown>

- Current version of `bedrock_server.exe` (1)
- Matching version of `bedrock_runtime_data` (2)
- The last version with debug info `bedrock_server.pdb` (3)

</div>

1. [Download from the official page](https://www.minecraft.net/en-us/download/server/bedrock)
2. [Published by LiteLDev here](https://github.com/LiteLDev/bedrock-runtime-data/releases)
3. [Download 1.21.3.01 (Windows)](https://www.minecraft.net/bedrockdedicatedserver/bin-linux/bedrock-server-1.21.3.01.zip)

## Extracting symbols from header

Enter the header file directory and configure

```
xmake f -m debug -p windows -a x64 --toolchain=clang --cxflags=-fplugin=/path/to/libdumpsym.so --sdk=/opt/msvc
```

Then, use DeThunk for preprocessing

!!! tips

    The path passed in should end with `mc` instead of `src` because it is not expected that the header files of `ll` should be traversed.

```
cd /path/to/scripts/dethunk
python main.py "/path/to/header/mc" --preset-extract-names --exclusion-list exclusion_list/r21_u6.toml
```

If there are no errors, start the build.

```
xmake build
```

If everything is OK, extract the symbol list from the directory below and copy it out.

```
build/.objs/bdsheader/windows/x64/debug/test/__cpp_main.cpp.cpp.symbols
```

The contents of the file look like this

```
CXXMethod, ?Submit@ThreadPool@OS@@QEAAXXZ
CXXConstructor, ??0WebviewDownloadInfo@@QEAA@XZ
CXXMethod, ??4WebviewDownloadInfo@@QEAAAEAU0@AEBU0@@Z
CXXMethod, ?getBatteryLevel@FakeBatteryMonitorInterface@@UEBA
...
```

## Extracting symbols from former PDB

As we all know, many functions in C++ programs are generated at compile time, so there are too few symbols that can be directly extracted from header files.
There is currently a lack of tools to directly generate "template symbols". (1)  
So we need to extract symbols from the previous PDB, use the following command
{ .annotate }

1. You may be interested in [this issue](https://github.com/liteldev-free/DebugInfo/issues/2).

```
./extractsym "/path/to/former/bedrock_server.pdb" -o 1.21.3.01.symbols
```

Similarly, copy this file out, its content is similar to...

```
Function, ??1I18nImpl@@UEAA@XZ
Function, ??_9PeerConnectionInterface@webrtc@@$BPI@AA
Function, ??1CommandArea@@QEAA@XZ
Var, ??_C@_0M@DPBEHEC@Destination@
...
```

## Extracting types from header

???+ warning "Optional step"

    This step is a bit complicated and unstable, and has some "workaround" meaning. If you can't figure it out, you can skip it.

First, you still need to use the header file, so restore it first.

```
cd /path/to/header/proj
git restore .
```

Then run DeThunk.

```
python main.py "/path/to/header/mc" --preset-extract-types --exclusion-list exclusion_list/r21_u6.toml
```

Then rebuild.

```
xmake build
```

After completion, you should be able to find `bdsheader.pdb` in the following path. In general it should be larger than 40M, copy it out.

```
build/windows/x64/release
```

## Generate symbol table

Since we just extracted the symbol lists from two different places, we pass them both to `askrva`.

!!! tip

    By the way, this will read bedrock_runtime_data from the current directory by default, make sure it exists.

```
./askrva __cpp_main.cpp.cpp.symbols 1.21.3.01.symbols -o succeed.json -of failed.txt
```

## Generate PDB

Now everything is ready, execute the following command to generate PDB.

!!! question

    You might be wondering why you are passing in bedrock_server.exe, this is so that the generated PDB matches the COFF file (so that the debugger can use it).

```
./makepdb --program bedrock_server.exe --symbol succeed.json --typeinfo bdsheader.pdb -o bedrock_server.pdb
```

## Finish

Everything is done! According to the information mentioned at the beginning of the article, this will generate at least 70M of PDB, which will be very useful to you, so enjoy using it!

If you have any questions, please join the community for discussion.
