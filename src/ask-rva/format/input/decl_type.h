#pragma once

namespace format::input {

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

    explicit DeclType(Enum value) : m_data(value) {}
    explicit DeclType(std::string_view str);

    bool isFunction() const { return m_data >= Function && m_data < Var; }
    bool isVar() const { return m_data >= Var && m_data < COUNT; }

    Enum data() const { return m_data; }

    bool operator==(const DeclType& other) const { return m_data == other.m_data; }

private:
    Enum m_data;
};

struct Symbol {
    std::string m_name;
    DeclType    m_type;

    bool operator==(const Symbol& other) const { return m_name == other.m_name; }
};

} // namespace format::input

namespace std {

template <>
struct hash<format::input::Symbol> {
    size_t operator()(const format::input::Symbol& symbol) const { return hash<string>{}(symbol.m_name); }
};

} // namespace std
