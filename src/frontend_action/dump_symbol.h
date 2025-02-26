#pragma once

#include <clang/Frontend/FrontendAction.h>

namespace clang {
class ASTConsumer;
class CompilerInstance;
} // namespace clang

namespace di::frontend_action {

class DumpSymbolFrontendAction : public clang::PluginASTAction {
protected:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& instance,
        llvm::StringRef
    ) override;

    bool ParseArgs(
        const clang::CompilerInstance&,
        const std::vector<std::string>& args
    ) override;

    ActionType getActionType() override;
};

} // namespace di::frontend_action
