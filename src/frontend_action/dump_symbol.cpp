#include "frontend_action/dump_symbol.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>

using namespace clang;

namespace {

bool config_record_decl_name = false;

class Container : private std::unordered_set<std::string> {
public:
    void put(const std::string& symbol) { emplace(symbol); }

    void write_to(const std::string& path) {
        std::ofstream ofs(path);
        if (ofs) {
            for (const auto& E : *this) {
                ofs << E << "\n";
            }
        }
    }
};

class Visitor : public RecursiveASTVisitor<Visitor> {
public:
    Visitor(ASTContext& context, Container& container)
    : m_namegen(context),
      m_symbol_container(container) {}

    bool VisitNamedDecl(NamedDecl* decl) {
        if (!decl || !decl->getDeclName()) return true;

        // FIXME: There are likely other contexts in which it makes
        // no sense to ask for a mangled name.
        if (isa<RequiresExprBodyDecl>(decl->getDeclContext())) return true;

        // If the declaration is dependent or is in a dependent
        // context, then the mangling is unlikely to be meaningful
        // (and in some cases may cause "don't know how to mangle
        // this" assertion failures.
        if (decl->isTemplated()) return true;

        // Mangled names are not meaningful for locals, and may not
        // be well-defined in the case of VLAs.
        auto* var_decl = dyn_cast<VarDecl>(decl);
        if (var_decl && var_decl->hasLocalStorage()) return true;

        // Do not mangle template deduction guides.
        if (isa<CXXDeductionGuideDecl>(decl)) return true;

        std::string mangled_name = m_namegen.getName(decl);
        if (!mangled_name.empty()) {
            if (config_record_decl_name) {
                mangled_name = std::format(
                    "{}, {}",
                    decl->getDeclKindName(),
                    mangled_name
                );
            }
            m_symbol_container.put(mangled_name);
        }

        return true;
    }

private:
    ASTNameGenerator m_namegen;
    Container&       m_symbol_container;
};

class Consumer : public ASTConsumer {
public:
    explicit Consumer(ASTContext&) {}

    void HandleTranslationUnit(ASTContext& context) override {
        Container symbols;

        Visitor(context, symbols)
            .TraverseDecl(context.getTranslationUnitDecl());

        // Save
        auto& source_manager = context.getSourceManager();
        auto  source_location =
            source_manager.getLocForStartOfFile(source_manager.getMainFileID());
        symbols.write_to(
            source_manager.getFilename(source_location).str() + ".symbols"
        );
    }
};

} // namespace

namespace di::frontend_action {

std::unique_ptr<ASTConsumer> DumpSymbolFrontendAction::CreateASTConsumer(
    CompilerInstance& instance,
    llvm::StringRef
) {
    return std::make_unique<Consumer>(instance.getASTContext());
}

bool DumpSymbolFrontendAction::ParseArgs(
    const CompilerInstance&,
    const std::vector<std::string>& args
) {
    for (const auto& arg : args) {
        if (arg.ends_with("record-decl-name")) {
            config_record_decl_name = true;
        }
    }
    return true;
}

PluginASTAction::ActionType DumpSymbolFrontendAction::getActionType() {
    return AddAfterMainAction;
}

} // namespace di::frontend_action
