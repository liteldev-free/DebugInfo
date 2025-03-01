#include "frontend_action/dump_symbol.h"

#include "data_format/typed_symbol_list.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>

using namespace clang;

using namespace di;
using namespace di::data_format;

namespace {

class Visitor : public RecursiveASTVisitor<Visitor> {
public:
    Visitor(ASTContext& context, TypedSymbolList& container)
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

        auto mangled_name = m_namegen.getName(decl);
        if (!mangled_name.empty()) {
            m_symbol_container.record(
                mangled_name,
                DeclType{decl->getDeclKindName()}
            );
        }

        return true;
    }

private:
    ASTNameGenerator m_namegen;
    TypedSymbolList& m_symbol_container;
};

class Consumer : public ASTConsumer {
public:
    explicit Consumer(ASTContext&) {}

    void HandleTranslationUnit(ASTContext& context) override {
        TypedSymbolList symbols;

        Visitor(context, symbols)
            .TraverseDecl(context.getTranslationUnitDecl());

        // Save
        auto& source_manager = context.getSourceManager();
        auto  source_location =
            source_manager.getLocForStartOfFile(source_manager.getMainFileID());
        symbols.write(
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

} // namespace di::frontend_action
