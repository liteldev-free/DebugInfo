# DumpSYM

This is a clang compiler plugin that outputs all declared symbols(and their types).

## Type of declaration

This depends on the two main base classes in LLVM: [FunctionDecl](https://clang.llvm.org/doxygen/classclang_1_1FunctionDecl.html), [VarDecl](https://clang.llvm.org/doxygen/classclang_1_1VarDecl.html). The general type name is the class name minus the "Decl" suffix.

See also `DeclType` for internal implementation details. (1)
{ .annotate }

1. https://github.com/liteldev-free/DebugInfo/blob/91bf6546da500b74186d26671d020c1b0a30c563/src/data_format/type/decl_type.h

## Compiler compatibility

- Only clang is supported. (GCC/MSVC are not supported)
- Only tested on Linux, not sure if it works on Windows.

## Usage

!!! note

    Both ItaniumABI and MSABI are supported, depending on the target passed to the compiler.

Simply pass `-fplugin=...` to clang and the plugin will run automatically.

???+ example

    ```
    clang++ -fplugin=/path/to/plugin/libdumpsym.so test.cpp
    ```

    The result will be generated in the `<TU>.symbols` file

    ```
    $ cat test.cpp.symbols
    Function, main
    CXXDestructor, ??_DThreadPool@OS@@QEAAXXZ
    CXXConstructor, ??0SpinLockImpl@@QEAA@AEBV0@@Z
    CXXMethod, ??4SpinLockImpl@@QEAAAEAV0@AEBV0@@Z
    Var, ?Low@OSThreadPriority@Threading@Bedrock@@2V123@B
    ```
