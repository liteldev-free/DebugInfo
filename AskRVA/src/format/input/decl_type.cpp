#include "format/input/decl_type.h"

#include "util/string.h"

namespace format::input {

DeclType::DeclType(std::string_view str) {
    using namespace util::string;

    // clang-format off

    switch (H(str)) {
#define HSTR(x)                                                                                                        \
    case H(#x):                                                                                                        \
        m_data = x;                                                                                                    \
        break
    HSTR(Function);
    HSTR(CXXDeductionGuide);
    HSTR(CXXMethod);
    HSTR(CXXConstructor);
    HSTR(CXXConversion);
    HSTR(CXXDestructor);
    HSTR(Var);
    HSTR(Decomposition);
    HSTR(ImplicitParam);
    HSTR(OMPCapturedExpr);
    HSTR(ParamVar);
    HSTR(VarTemplateSpecialization);
#undef HSTR
    default:
        throw std::invalid_argument("Unexpected decl type.");
    }

    // clang-format on
}

} // namespace format::input
