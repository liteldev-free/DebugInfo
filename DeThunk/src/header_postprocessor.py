import util.cpp_language as CppUtil


_need_fix_includes_queue = dict()
_need_fix_members_queue = dict()
_class_defs_record = dict()


class ClassDefineRecord:
    rpath = str()
    is_template = bool()
    is_empty = bool()

    def __init__(self, rpath: str, is_template: bool, is_empty: bool):
        self.rpath = rpath
        self.is_template = is_template
        self.is_empty = is_empty


def record_class_definition(
    path: str, namespace: str, class_name: str, is_template: bool, is_empty: bool
):
    assert len(path) > 0 and len(class_name) > 0
    assert '::' not in class_name  # c++ does not support forward declaration for nested class.
    if namespace not in _class_defs_record:
        _class_defs_record[namespace] = {}
    assert class_name not in _class_defs_record[namespace], (
        f'path = {path}, ns = {namespace}, cl = {class_name}'
    )
    _class_defs_record[namespace][class_name] = ClassDefineRecord(
        path[path.find('src/') + 4 :], is_template, is_empty
    )


def add_pending_fix_includes_queue(path: str, decls: list, member_typeset: list):
    assert path not in _need_fix_includes_queue
    if len(decls) > 0 and len(member_typeset) > 0:
        _need_fix_includes_queue[path] = [decls, member_typeset]


def add_pending_fix_members_queue(path: str, member_typeset: list):
    assert path not in _need_fix_members_queue
    if len(member_typeset) > 0:
        _need_fix_members_queue[path] = member_typeset


def _find_definition(decl: str, in_types: list) -> ClassDefineRecord | None:
    find_decl = CppUtil.find_class_forward_declaration(decl)
    assert find_decl

    namespace = find_decl.namespace_decl
    clazz = find_decl.class_decl

    if not CppUtil.is_full_type_required_for_typeset(namespace, clazz, in_types):
        return None

    assert namespace in _class_defs_record, f'namespace not recorded, {namespace}'
    assert clazz in _class_defs_record[namespace], f'{clazz} not recorded, in {namespace}'

    return _class_defs_record[namespace][find_decl.class_decl]


def process():
    # fix includes
    for path, decl_and_types in _need_fix_includes_queue.items():
        with open(path, 'r', encoding='utf-8') as file:
            content = file.read()
            for decl in decl_and_types[0]:
                record = _find_definition(decl, decl_and_types[1])
                if record:
                    include = f'#include "{record.rpath}"'
                    content = content.replace(decl, include)
            with open(path, 'w', encoding='utf-8') as wfile:
                wfile.write(content)
    # fix members
    # for path, types in _need_fix_members_queue.items():
    #    with open(path, 'r', encoding='utf-8') as file:
    #        content = file.read()
    #        with open(path, 'w', encoding='utf-8') as wfile:
    #            wfile.write(content)
