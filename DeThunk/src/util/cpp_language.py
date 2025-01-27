"""
C++ Language Utility
 * some methods may not be designed to be universal.
"""


class ForwardDeclaration:
    namespace_decl = str()
    class_decl = str()

    def __init__(self, namespace_decl: str, class_decl: str):
        self.namespace_decl = namespace_decl
        self.class_decl = class_decl


def is_header_file(path: str):
    return path.endswith('.h') or path.endswith('.hpp')


def find_class_definition(line: str) -> str | None:
    # class A (no quotation mark)
    # class A :
    # class A {
    # class A { ... }; (in single line)

    specifier_size = len('class')
    class_pos = line.find('class ')
    struct_pos = line.find('struct ')
    assert class_pos == -1 or struct_pos == -1, f'line = {line}, c = {class_pos}, s = {struct_pos}'

    left_brace_pos = line.find('{')
    semicolon_pos = line.find(';')
    if semicolon_pos != -1 and (left_brace_pos == -1 or semicolon_pos < left_brace_pos):
        return None  # is forward decl
    if class_pos == -1:
        if struct_pos == -1:
            return None  # is not class defs
        specifier_size = len('struct')
        class_pos = struct_pos

    end_pos = len(line)
    colon_pos = line.find(':')
    if left_brace_pos != -1:
        end_pos = min(end_pos, left_brace_pos)
    if colon_pos != -1:
        end_pos = min(end_pos, colon_pos)

    return line[class_pos + specifier_size : end_pos].strip()


def find_class_forward_declaration(line: str) -> ForwardDeclaration | None:
    # class A;
    # namespace B { class A; }

    namespace_decl = ''
    class_decl = ''

    namespace_pos = line.find('namespace')
    left_brace_pos = line.find('{')
    if namespace_pos != -1 and left_brace_pos != -1:
        namespace_decl = line[namespace_pos + len('namespace') : left_brace_pos].strip()

    specifier_size = len('class')
    class_pos = line.find('class ')
    struct_pos = line.find('struct ')
    assert class_pos == -1 or struct_pos == -1

    semicolon_pos = line.find(';')
    if semicolon_pos == -1:
        return None
    if class_pos == -1:
        if struct_pos == -1:
            return None
        specifier_size = len('struct')
        class_pos = struct_pos

    class_decl = line[class_pos + specifier_size : semicolon_pos].strip()
    return ForwardDeclaration(namespace_decl, class_decl)


def find_namespace_declaration(line: str) -> str | None:
    namespace_pos = line.find('namespace')
    left_brace_pos = line.find('{')
    if namespace_pos == -1 or left_brace_pos == -1:
        return None

    return line[namespace_pos + len('namespace') : left_brace_pos].strip()
