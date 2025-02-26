#include "action/dump_symbol.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>

using namespace clang;

namespace {

bool cfg_RecordDeclName = false;

class Container : private std::unordered_set<std::string> {
public:
    void Put(const std::string& Symbol) { emplace(Symbol); }

    void WriteTo(const std::string& Path) {
        std::ofstream OFS(Path);
        for (const auto& E : *this) {
            OFS << E << "\n";
        }
    }
};

class Visitor : public RecursiveASTVisitor<Visitor> {
public:
    Visitor(ASTContext& Context, Container& Container)
    : ASTNameGen(Context),
      Symbols(Container) {}

    bool VisitNamedDecl(NamedDecl* ND) {
        if (!ND || !ND->getDeclName()) return true;

        // FIXME: There are likely other contexts in which it makes
        // no sense to ask for a mangled name.
        if (isa<RequiresExprBodyDecl>(ND->getDeclContext())) return true;

        // If the declaration is dependent or is in a dependent
        // context, then the mangling is unlikely to be meaningful
        // (and in some cases may cause "don't know how to mangle
        // this" assertion failures.
        if (ND->isTemplated()) return true;

        // Mangled names are not meaningful for locals, and may not
        // be well-defined in the case of VLAs.
        auto* VD = dyn_cast<VarDecl>(ND);
        if (VD && VD->hasLocalStorage()) return true;

        // Do not mangle template deduction guides.
        if (isa<CXXDeductionGuideDecl>(ND)) return true;

        std::string MangledName = ASTNameGen.getName(ND);
        if (!MangledName.empty()) {
            if (cfg_RecordDeclName) {
                MangledName =
                    std::format("{}, {}", ND->getDeclKindName(), MangledName);
            }
            Symbols.Put(MangledName);
        }

        return true;
    }

private:
    ASTNameGenerator ASTNameGen;
    Container&       Symbols;
};

class Consumer : public ASTConsumer {
public:
    explicit Consumer(ASTContext& Context) {}

    void HandleTranslationUnit(ASTContext& Context) override {
        Container Symbols;

        Visitor(Context, Symbols)
            .TraverseDecl(Context.getTranslationUnitDecl());

        // Save
        auto& SM  = Context.getSourceManager();
        auto  Loc = SM.getLocForStartOfFile(SM.getMainFileID());
        Symbols.WriteTo(SM.getFilename(Loc).str() + ".symbols");
    }
};

} // namespace

std::unique_ptr<ASTConsumer> DumpSymbolFrontendAction::CreateASTConsumer(
    CompilerInstance& CI,
    llvm::StringRef
) {
    return std::make_unique<Consumer>(CI.getASTContext());
}

bool DumpSymbolFrontendAction::ParseArgs(
    const CompilerInstance&         CI,
    const std::vector<std::string>& args
) {
    for (const auto& arg : args) {
        if (arg.ends_with("record-decl-name")) {
            cfg_RecordDeclName = true;
        }
    }
    return true;
}

PluginASTAction::ActionType DumpSymbolFrontendAction::getActionType() {
    return AddAfterMainAction;
}
