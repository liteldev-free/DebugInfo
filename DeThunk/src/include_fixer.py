import os
import re

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

        if not _is_full_type_needed(namespace, clazz, in_types):
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


def _is_full_type_needed(namespace_decl: str, class_decl: str, in_types: list):
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

    def find_template_name(full: str, what: str):
        for matched in re.finditer(rf'\b{re.escape(what)}\b', full):
            endpos = matched.start()
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

        return None

    for type_name in in_types:
        founded, endswith = is_subtk_ends_with(
            type_name, class_decl, ['&', '*', ' const&', ' const*']
        )
        # is not reference or pointer type
        if founded and not endswith:
            # is template parameter?
            template_name = find_template_name(type_name, class_decl)
            if template_name:
                if template_name in [  # NOT Need full type.
                    'map',
                    'shared_ptr',
                    'unique_ptr',
                    'weak_ptr',
                    'vector',
                    'queue',
                    'set',
                    'function',
                ]:
                    pass  # don't return false directly
                elif template_name in [  # Need full type.
                    'optional',
                    'variant',
                    'array',
                    'pair',
                    'unordered_set',
                    'deque',
                    'queue',
                ]:
                    return True
                elif template_name in [  # EMPTY TEMPLATE CLASS
                    'ScriptFilteredEventSignal',
                    'OwnerPtr',
                    'UniqueOwnerPointer',
                    'NotNullNonOwnerPtr',
                    'NonOwnerPointer',
                    'ServiceRegistrationToken',
                    'IDType',
                    'WeakRef',
                    'SubChunkStorage',
                    'ServiceReference',
                    'typeid_t',
                    'List',
                    'MemoryPool',
                    'ThreadOwner',
                    'StrongTypedObjectHandle',
                    'Promise',
                    'Factory',
                    'Publisher',
                    'ServiceRegistrationToken',
                ]:
                    pass
                else:
                    return True  # on default
            else:
                return True  # not a template parameter

    return False
