#pragma once

#include "util/string.h"

namespace di {

class DeclType {
public:
    enum Enum {
        Function,
        CXXDeductionGuide,
        CXXMethod,
        CXXConstructor,
        CXXConversion,
        CXXDestructor,

        Var,
        Decomposition,
        ImplicitParam,
        OMPCapturedExpr,
        ParamVar,
        VarTemplateSpecialization,

        COUNT
    };

    constexpr DeclType(Enum value) : m_data(value) {}
    constexpr explicit DeclType(std::string_view str) {
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
            throw EnumTransformException(str, Enum{});
        }

        // clang-format on
    }

    constexpr bool is_function() const {
        return m_data >= Function && m_data < Var;
    }
    constexpr bool is_variable() const {
        return m_data >= Var && m_data < COUNT;
    }

    constexpr Enum data() const { return m_data; }

    constexpr std::string string() const {
        using namespace util::string;

        // clang-format off

        switch (m_data) {
    #define HSTR(x)                                                                \
        case x:                                                                    \
            return #x;
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
            throw EnumTransformException(m_data, Enum{});
        }

        // clang-format on
    }

    constexpr bool operator==(const DeclType& other) const {
        return m_data == other.m_data;
    }

private:
    Enum m_data;
};

} // namespace di
