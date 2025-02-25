# DumpSYM

Sometimes, we need to extract symbols from declarations. So this compiler plugin was born.

### Build

> As far as I know, there are some problems with the clang plugin executing under Windows, so I recommend that all operations be performed under Linux.

- Building llvm will consume a lot of time and resources, it is recommended to pre-install llvm from your system package manager, xmake can detect system packages.
- Run `xmake` to build.

### Usage

Simply pass `-fplugin=...` to clang and the plugin will run automatically.

#### Optional Arguments

- `record-decl-name` - Add the name of the Decl in the output, reference: [FunctionDecl](https://clang.llvm.org/doxygen/classclang_1_1FunctionDecl.html), [VarDecl](https://clang.llvm.org/doxygen/classclang_1_1VarDecl.html)

> [!NOTE]
> Because LLVM is used, both ItaniumABI and MicrosoftABI are supported.

- Example:

```
$ clang++ -fplugin=/path/to/plugin/libdumpsym.so -fplugin-arg-dumpsym-record-decl-name test.cpp
```

- The result will be generated in the `<TU>.symbols` file

```
$ cat test.cpp.symbols
Function, main
CXXDestructor, ??_DThreadPool@OS@@QEAAXXZ
CXXConstructor, ??0SpinLockImpl@@QEAA@AEBV0@@Z
CXXMethod, ??4SpinLockImpl@@QEAAAEAV0@AEBV0@@Z
Var, ?Low@OSThreadPriority@Threading@Bedrock@@2V123@B
```
