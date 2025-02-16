import os

import util.cpp_language as CppUtil
import util.string as StrUtil

# storage
defined_classes = dict()
empty_class_all_names = set()


class ClassDefine:
    rpath = str()
    is_template = bool()
    is_empty = bool()

    def __init__(self, rpath: str, is_template: bool, is_empty: bool):
        self.rpath = rpath
        self.is_template = is_template
        self.is_empty = is_empty


def add_class_record(path: str, namespace: str, class_name: str, is_template: bool, is_empty: bool):
    assert len(path) > 0 and len(class_name) > 0
    if namespace not in defined_classes:
        defined_classes[namespace] = {}
    assert class_name not in defined_classes[namespace], (
        f'path = {path}, ns = {namespace}, cl = {class_name}'
    )
    print(f'path = {path}, ns = {namespace}, cl = {class_name}')
    defined_classes[namespace][class_name] = ClassDefine(
        path[path.find('src/') + 4 :], is_template, is_empty
    )

    if is_empty:
        all_name = ''
        if namespace != '':
            all_name += namespace + '::'
        all_name += class_name
        empty_class_all_names.add(all_name)


def query_class_record_strict(namespace_decl: str, class_decl: str) -> ClassDefine:
    assert namespace_decl in defined_classes, f'namespace = "{namespace_decl}" is not recorded.'
    assert class_decl in defined_classes[namespace_decl], (
        f'namespace = "{namespace_decl}", class = {class_decl} is not recorded.'
    )

    return defined_classes[namespace_decl][class_decl]


def process(path_to_file: str):
    assert os.path.isfile(path_to_file)

    if path_to_file.endswith('_HeaderOutputPredefine.h'):
        return

    with open(path_to_file, 'r', encoding='utf-8') as file:
        # states
        in_forward_declaration_list = False
        current_namespace = []
        current_classes = []

        # tmp
        content = ''
        for line in file.readlines():
            stripped_line = line.strip()

            # record forward declarations
            if stripped_line.startswith('// auto generated forward declare list'):
                in_forward_declaration_list = True
            if stripped_line.startswith('// clang-format on') and in_forward_declaration_list:
                in_forward_declaration_list = False

            # record namespace & classes
            if not in_forward_declaration_list:
                founded_cl = CppUtil.find_class_definition(line)
                if founded_cl:  # ignore anonymous classes.
                    is_template = (
                        content[content.rfind('\n', 0, -1) :].strip().startswith('template ')
                    )
                    is_empty = stripped_line.endswith('{};')
                    nested_cl = ''
                    if current_classes:
                        for pair in current_classes:
                            nested_cl += pair[1] + '::'
                    add_class_record(
                        path_to_file,
                        '::'.join(current_namespace),
                        nested_cl + founded_cl,
                        is_template,
                        is_empty,
                    )
                    class_keyword_pos, _ = StrUtil.find_m(line, 'class ', 'struct ', 'union ')
                    assert class_keyword_pos != -1, f"path = {path_to_file}, line = '{line}'"

                    current_classes.append([class_keyword_pos, founded_cl])
                founded_ns = CppUtil.find_namespace_declaration(line)
                if founded_ns:
                    current_namespace.append(founded_ns)
                if '// namespace' in stripped_line:
                    current_namespace.pop()

            if current_classes and line.startswith(' ' * (current_classes[-1][0]) + '};'):
                current_classes.pop()
