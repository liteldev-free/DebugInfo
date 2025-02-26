#include "action/dump_symbol.h"

#include <clang/Frontend/FrontendPluginRegistry.h>

using namespace clang;

static FrontendPluginRegistry::Add<DumpSymbolFrontendAction>
    X("dumpsym", "Extract all declared symbols from a TU.");
