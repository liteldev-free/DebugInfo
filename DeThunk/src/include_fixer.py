import os

import util.cpp_language as CppUtil


class IncludeFixer:
    _need_fix_queue = dict()
    _class_defs_record = dict()

    def __init__(self):
        pass

    def record_class_definition(self, path: str, namespace: str, class_name: str):
        assert len(path) > 0 and len(class_name) > 0
        assert '::' not in class_name  # c++ does not support forward declaration for nested class.
        if namespace not in self._class_defs_record:
            self._class_defs_record[namespace] = {}
        assert class_name not in self._class_defs_record[namespace]
        self._class_defs_record[namespace][class_name] = path[path.find('src/') + 4 :]

    def add_pending_fix_queue(self, path: str, decls: list, member_typeset: list):
        assert os.path.isfile(path)
        assert path not in self._need_fix_queue
        if len(decls) > 0 and len(member_typeset):
            self._need_fix_queue[path] = [decls, member_typeset]

    def _find_definition(self, decl: str, in_types: list):
        find_decl = CppUtil.find_class_forward_declaration(decl)
        assert find_decl

        namespace = find_decl.namespace_decl
        clazz = find_decl.class_decl

        if not CppUtil.is_full_type_required_for_typeset(namespace, clazz, in_types):
            return None

        assert namespace in self._class_defs_record, f'namespace not recorded, {namespace}'
        assert clazz in self._class_defs_record[namespace], f'{clazz} not recorded, in {namespace}'

        return self._class_defs_record[namespace][find_decl.class_decl]

    def run_fix(self):
        for path, decl_and_types in self._need_fix_queue.items():
            with open(path, 'r', encoding='utf-8') as file:
                content = file.read()
                for decl in decl_and_types[0]:
                    define_location = self._find_definition(decl, decl_and_types[1])
                    if define_location:
                        include = f'#include "{define_location}"'
                        content = content.replace(decl, include)
                with open(path, 'w', encoding='utf-8') as wfile:
                    wfile.write(content)
