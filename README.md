# DebugInfo
This repository regenerates debug information (like PDB) from LeviLamina's public data.

## Background
 - After 1.21.3.01, Mojang removed debug information from BDS.
 - Mojang no longer provides any debugging information (both server and client) to the community.
 - Mojang has an agreement with LiteLDev to provide them with debug data.
 - LiteLDev generates header files and obfuscated symbol to RVA lookup tables from debug data and provides them to the community.

### Problems caused by Mojang's collaboration with LiteLDev
 - Mojang's collaboration with LiteLDev is opaque and we have no idea what they do.
 - LiteLDev has completed its monopoly, and there will no longer be a second mod loader in the community.
 - Due to the obfuscated format, the community can no longer reverse engineer BDS.

### Header files, obfuscation format and security
 - LeviLamina's design necessitates that they publish header files.
 - The header file contains all the declaration information so that symbols can be generated.
 - The RVA of the corresponding symbol can be extracted from the obfuscated format at runtime.
 - The obfuscated format is actually a carrier of the complete "symbol table", which used to be PDB/DWARF.

## Tool for restoring original DebugInfo from obfuscated format
> [!NOTE]
> LiteLDev has not yet released bedrock_runtime_data/magicblob for the Linux server.

They are dethunk, dumpsym, askrva and makepdb. Each tool is in a directory with the same name as it, and also has a README to help you use it.  
In short, the PDB is generated by the following steps:
 - Preprocess the header files published by LiteLDev by dethunk.
```
python main.py {HEADER_PROJECT_DIR}/src
```
 - Compile the header file and load the dumpsym plugin in the compilation parameters.
```
xmake f -c -p windows -a x64 -m release --sdk=/opt/msvc --cxflags="-fplugin=/path/to/libdumpsym.so -fplugin-arg-dumpsym-record-decl-name" --toolchain=clang
xmake -v
```
 - Find the generated symbols file.
```
{HEADER_PROJECT_DIR}/build/.objs/bdsheader/windows/x64/release/test/__cpp_main.cpp.cpp.symbols
```
 - Generate symbol table using askrva.
```
./askrva __cpp_main.cpp.cpp.symbols --output succeed.json --output-failed failed.txt --output-format=makepdb
```
 - Generate PDB using makepdb.
```
./makepdb --program bedrock_server.exe --symbol-data succeed.json --output bedrock_server.pdb
```

## TODO
 - [ ] Tap into more available symbols.
 - [ ] Fully open source HeaderGen.
 - [ ] Bootstrap.
 - [ ] Opti project structure.

## Be with us
Our vision is to build an open and inclusive Minecraft: Bedrock Edition ecosystem.
 - [https://t.me/bdsplugins](https://t.me/s/bdsplugins)

## LICENSE
All tools are open source under the MIT license.
