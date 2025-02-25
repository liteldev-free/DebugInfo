"""
C++ Language Utility
 * some methods may not be designed to be universal.
"""

import re

import util.string as StrUtil


def is_header_file(path: str):
    return path.endswith('.h') or path.endswith('.hpp')


def find_class_definition(line: str) -> str | None:
    # KEYWORD A (no quotation mark)
    # KEYWORD A :
    # KEYWORD A {
    # KEYWORD A { ... }; (in single line)

    keyword_pos, keyword = StrUtil.find_mb(line, 'enum class ', 'class ', 'struct ', 'union ')
    if keyword_pos == -1 or keyword == 'enum class ':
        return None

    keyword_size = len(keyword) - 1  # not class defs

    left_brace_pos = line.find('{')
    semicolon_pos = line.find(';')
    if semicolon_pos != -1 and (left_brace_pos == -1 or semicolon_pos < left_brace_pos):
        return None  # is forward decl

    end_pos = len(line)
    colon_pos = line.find(':')
    l_angle_bracket_pos = line.find('<')
    if left_brace_pos != -1:
        end_pos = min(end_pos, left_brace_pos)
    if colon_pos != -1:
        end_pos = min(end_pos, colon_pos)
        if l_angle_bracket_pos != -1 and l_angle_bracket_pos < colon_pos:
            return None  # template specialization (is not supported)

    result = line[keyword_pos + keyword_size : end_pos].strip()
    if '>' in result:
        return None

    return result


class ForwardDeclaration:
    namespace_decl = str()
    class_decl = str()

    def __init__(self, namespace_decl: str, class_decl: str):
        self.namespace_decl = namespace_decl
        self.class_decl = class_decl


def find_class_forward_declaration(line: str) -> ForwardDeclaration | None:
    # class A;
    # namespace B { class A; }

    namespace_decl = ''
    class_decl = ''

    namespace_pos = line.find('namespace')
    left_brace_pos = line.find('{')
    if namespace_pos != -1 and left_brace_pos != -1:
        namespace_decl = line[namespace_pos + len('namespace') : left_brace_pos].strip()

    keyword_pos, keyword = StrUtil.find_mb(line, 'class ', 'struct ', 'union ')
    if keyword_pos == -1:
        return None

    keyword_size = len(keyword) - 1  # not class defs

    semicolon_pos = line.find(';')
    if semicolon_pos == -1:
        return None

    class_decl = line[keyword_pos + keyword_size : semicolon_pos].strip()
    return ForwardDeclaration(namespace_decl, class_decl)


def find_namespace_declaration(line: str) -> str | None:
    namespace_pos = line.find('namespace')
    left_brace_pos = line.find('{')
    if namespace_pos == -1 or left_brace_pos == -1:
        return None

    return line[namespace_pos + len('namespace') : left_brace_pos].strip()


def find_template_name(full: str, what: str, disable_regex_word_bound: bool = False):
    def _impl(endpos: int):
        while True:
            r_angle_bracket_pos = full.rfind('>', 0, endpos)
            l_angle_bracket_pos = full.rfind('<', 0, endpos)
            if l_angle_bracket_pos == -1:
                return None
            if r_angle_bracket_pos > l_angle_bracket_pos:
                endpos = l_angle_bracket_pos
                continue
            ret = full[:l_angle_bracket_pos]
            matched_non_name = list(re.finditer(r'[^a-zA-Z_]', ret))
            if len(matched_non_name) > 0:
                ret = ret[matched_non_name[-1].start() + 1 :]
            assert len(ret) > 0
            return ret

    if not disable_regex_word_bound:
        for matched in re.finditer(rf'\b{re.escape(what)}\b', full):
            endpos = matched.start()
            result = _impl(endpos)
            if result:
                return result
    else:
        endpos = full.find(what)
        if endpos == -1:
            return None
        return _impl(endpos)


def is_full_type_required(namespace_decl: str, class_decl: str, type_decl: str):
    """
    Determine whether `class_decl` requires a full type for `type_decl`.

    TODO: namespace_decl (currently UNUSED).
    """

    # Y: T
    # Y: std::optional<T>
    # Y: std::variant<T>
    # Y: std::array<T, _>
    # Y: std::pair<T, T>
    # Y: std::unordered_set<T>
    # Y: std::unordered_map<T, _>
    # Y: std::deque<T>  // under msstl only
    # Y: std::queue<T>  // under msstl only
    # N: T&
    # N: T*
    # N: std::map<T, T>
    # N: std::shared_ptr<T>
    # N: std::unique_ptr<T>
    # N: std::weak_ptr<T>
    # N: std::vector<T>
    # N: std::set<T>
    # N: std::unordered_map<_, T>
    # N: std::function<T(T)>

    def is_subtk_ends_with(full: str, tk: str, whats: list):
        founded = False
        for matched in re.finditer(rf'\b{re.escape(tk)}\b', full):
            founded = True
            if len(full) > matched.end():
                for what in whats:
                    if full[matched.end() : matched.end() + len(what)] == what:
                        return founded, True
        return founded, False

    # is reference or pointer type?
    founded, is_endswith = is_subtk_ends_with(
        type_decl, class_decl, ['&', '*', ' const&', ' const*']
    )

    if not founded or is_endswith:
        return False

    # is template params?
    template_name = find_template_name(type_decl, class_decl)
    if not template_name:
        return True  # moreover, is not a template parameter

    if template_name in [  # forward declarations are allowed.
        'map',
        'shared_ptr',
        'unique_ptr',
        'weak_ptr',
        # 'vector',
        'queue',
        'set',
        'function',
    ]:
        return False
    elif template_name in [  # full type is required.
        'optional',
        'variant',
        'array',
        'pair',
        'unordered_set',
        'deque',
        'queue',
    ]:
        return True
    else:
        return True  # by default, we assume that a full type is required.


def is_full_type_required_for_typeset(namespace_decl: str, class_decl: str, typeset: list):
    for type_decl in typeset:
        if is_full_type_required(namespace_decl, class_decl, type_decl):
            return True
    return False
