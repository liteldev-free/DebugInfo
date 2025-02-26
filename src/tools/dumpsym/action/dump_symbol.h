#pragma once

#include <clang/Frontend/FrontendAction.h>

namespace clang {
class ASTConsumer;
class CompilerInstance;
} // namespace clang

class DumpSymbolFrontendAction : public clang::PluginASTAction {
protected:
    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance& CI, llvm::StringRef) override;

    bool ParseArgs(
        const clang::CompilerInstance&  CI,
        const std::vector<std::string>& args
    ) override;

    ActionType getActionType() override;
};