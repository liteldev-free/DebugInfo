import os

import util.cpp_language as CppUtil
import util.string as StrUtil

# storage
defined_classes = dict()


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
    assert '::' not in class_name  # c++ does not support forward declaration for nested class.
    if namespace not in defined_classes:
        defined_classes[namespace] = {}
    assert class_name not in defined_classes[namespace], (
        f'path = {path}, ns = {namespace}, cl = {class_name}'
    )
    defined_classes[namespace][class_name] = ClassDefine(
        path[path.find('src/') + 4 :], is_template, is_empty
    )


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
                if StrUtil.startswith_m(line, 'class ', 'struct ', 'union '):  # ignore nested class
                    founded = CppUtil.find_class_definition(line)
                    if founded:
                        is_template = (
                            content[content.rfind('\n', 0, -1) :].strip().startswith('template ')
                        )
                        is_empty = stripped_line.endswith('{};')
                        add_class_record(
                            path_to_file,
                            '::'.join(current_namespace),
                            founded,
                            is_template,
                            is_empty,
                        )
                founded = CppUtil.find_namespace_declaration(line)
                if founded:
                    current_namespace.append(founded)
                if '// namespace' in stripped_line:
                    current_namespace.pop()
