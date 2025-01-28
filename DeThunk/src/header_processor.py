import os
import re

import header_preprocessor as HeaderPreProcessor
import header_postprocessor as HeaderPostProcessor

import util.cpp_language as CppUtil


class Options:
    base_dir = str()

    # functions
    remove_constructor_thunk = bool()
    remove_destructor_thunk = bool()
    remove_virtual_function_thunk = bool()

    # variables
    remove_virtual_table_pointer_thunk = bool()
    restore_static_variable = bool()
    restore_member_variable = bool()

    # others
    # * only takes effect for TypedStorage, since the TypedStorage wrapper makes the full type unnecessary.
    fix_includes_for_member_variables = True
    # * template definitions cannot be generated for headergen and may be wrong. class sizes may be wrong if
    # * empty templates are used in TypedStorage.
    # * this option will erase the type of the empty template (convert to uchar[size]).
    fix_size_for_type_with_empty_template_class = True
    # * this option will and add sizeof & alignof static assertions to members. (only takes effect for TypedStorage)
    add_sizeof_alignof_static_assertions = True

    def __init__(self, args):
        self.base_dir = args.path
        self.remove_constructor_thunk = args.remove_constructor_thunk
        self.remove_destructor_thunk = args.remove_destructor_thunk
        self.remove_virtual_function_thunk = args.remove_virtual_function_thunk
        self.remove_virtual_table_pointer_thunk = args.remove_virtual_table_pointer_thunk
        self.restore_static_variable = args.restore_static_variable
        self.restore_member_variable = args.restore_member_variable

        if args.all:
            self.set_all(True)

    def set_function(self, opt: bool):
        self.remove_constructor_thunk = opt
        self.remove_destructor_thunk = opt
        self.remove_virtual_function_thunk = opt

    def set_variable(self, opt: bool):
        self.remove_virtual_table_pointer_thunk = opt
        self.restore_static_variable = opt
        self.restore_member_variable = opt

    def set_all(self, opt: bool):
        self.set_function(opt)
        self.set_variable(opt)


def process(path_to_file: str, args: Options):
    assert os.path.isfile(path_to_file)

    if path_to_file.endswith('_HeaderOutputPredefine.h'):
        return

    RECORDED_THUNKS = []
    if args.remove_constructor_thunk:
        RECORDED_THUNKS.append('// constructor thunks')
    if args.remove_destructor_thunk:
        RECORDED_THUNKS.append('// destructor thunk')
    if args.remove_virtual_table_pointer_thunk:
        RECORDED_THUNKS.append('// vftables')
    if args.remove_virtual_function_thunk:
        RECORDED_THUNKS.append('// virtual function thunks')

    with open(path_to_file, 'r', encoding='utf-8') as file:
        # states
        is_modified = False
        in_useless_thunk = False
        in_static_variable = False
        in_member_variable = False
        in_forward_declaration_list = False
        has_typed_storage = False
        current_namespace = []
        forward_declarations = []
        member_variable_types = []

        ll_typed_regex = re.compile(r'TypedStorage<(\d+), (\d+), (.*?)> (\w+);')
        ll_untyped_regex = re.compile(r'UntypedStorage<(\d+), (\d+)> (\w+);')

        def regex_preprocess_name(con: str):
            # typed storage can be very complex, so we need to preprocess it.
            # TODO: find a better way.
            return re.sub(r'\s+', ' ', con).replace('< ', '<')

        # tmp
        content = ''
        for line in file.readlines():
            stripped_line = line.strip()

            # restore static member variable:
            if args.restore_static_variable and '// static variables' in line:
                in_static_variable = True
                is_modified = True
            if in_static_variable and stripped_line.endswith(';'):
                if not stripped_line.startswith('MCAPI'):  # declaration may not be on one line
                    begin_pos = content.rfind('MCAPI')
                    stripped_line = content[begin_pos:] + stripped_line
                    content = content[:begin_pos]
                    stripped_line = stripped_line.strip()

                # remove parameter list (convert to static variable)
                call_spec_pos = stripped_line.rfind('()')
                assert call_spec_pos != -1

                stripped_line = stripped_line[:call_spec_pos] + stripped_line[call_spec_pos + 2 :]

                # remove reference
                refsym_pos = stripped_line.rfind('&')  # T&
                tlpsym_pos = stripped_line.rfind('>')  # ::std::add_lvalue_reference_t<T>
                assert refsym_pos != -1 or tlpsym_pos != -1, f'in {path_to_file}'
                if tlpsym_pos == -1 or refsym_pos > tlpsym_pos:
                    stripped_line = stripped_line[:refsym_pos] + stripped_line[refsym_pos + 1 :]
                elif refsym_pos == -1 or tlpsym_pos > refsym_pos:
                    # C-style arrays must have '[]' written after the variable name
                    stripped_line = stripped_line[:tlpsym_pos] + '>' + stripped_line[tlpsym_pos:]
                    stripped_line = stripped_line.replace(
                        '::std::add_lvalue_reference_t<',
                        '::std::remove_reference_t<::std::add_lvalue_reference_t<',
                    )

                content += f'\t{stripped_line}\n'
                continue

            # restore member variable:
            if args.restore_member_variable and stripped_line.startswith(
                '::ll::'
            ):  # union { ... };
                in_member_variable = True
                is_modified = True
            if in_member_variable and stripped_line.endswith(';'):
                if not stripped_line.startswith('::ll::'):
                    begin_pos = content.rfind('::ll::')
                    stripped_line = content[begin_pos:] + stripped_line
                    content = content[:begin_pos]
                    stripped_line = stripped_line.strip()

                # ::ll::TypedStorage<Alignment, Size, T> mVar;
                if 'TypedStorage' in stripped_line:
                    has_typed_storage = True
                    matched = ll_typed_regex.search(regex_preprocess_name(stripped_line))
                    assert matched and matched.lastindex == 4, (
                        f'in {path_to_file}, line="{stripped_line}"'
                    )

                    align = matched[1]  # unused.
                    size = matched[2]  # unused.
                    type_name = matched[3]
                    var_name = matched[4]

                    if type_name.endswith(']'):  # is c-style array
                        array_length = int(type_name[type_name.find('[') + 1 : type_name.find(']')])
                        type_name = type_name[: type_name.find('[')]
                        var_name = f'{var_name}[{array_length}]'

                    fun_ptr_pos = type_name.find('(*)')
                    if -1 != fun_ptr_pos:  # is c-style function ptr
                        type_name = (
                            type_name[: fun_ptr_pos + 2] + var_name + type_name[fun_ptr_pos + 2 :]
                        )
                        var_name = ''

                    content += f'\t{type_name} {var_name};\n'

                    member_variable_types.append(type_name)
                    in_member_variable = False
                    continue

                # ::ll::UntypedStorage<Alignment, Size>  mVar;
                if 'UntypedStorage' in stripped_line:
                    matched = ll_untyped_regex.search(regex_preprocess_name(stripped_line))
                    assert matched and matched.lastindex == 3, (
                        f'in {path_to_file}, line="{stripped_line}"'
                    )

                    align = matched[1]
                    size = matched[2]
                    var_name = matched[3]

                    content += f'\talignas({align}) std::byte {var_name}[{size}];\n'

                    in_member_variable = False
                    continue

                assert False, 'unreachable'

            # record forward declarations
            if stripped_line.startswith('// auto generated forward declare list'):
                in_forward_declaration_list = True
            if in_forward_declaration_list:
                if (
                    stripped_line.startswith('class ')
                    or stripped_line.startswith('struct ')
                    or stripped_line.startswith('namespace ')
                ):
                    forward_declarations.append(stripped_line)
            if stripped_line.startswith('// clang-format on') and in_forward_declaration_list:
                in_forward_declaration_list = False

            # record namespace & classes
            if not in_forward_declaration_list:
                if line.startswith('class ') or line.startswith('struct '):  # ignore nested class
                    founded = CppUtil.find_class_definition(line)
                    if founded:
                        is_template = (
                            content[content.rfind('\n', 0, -1) :].strip().startswith('template ')
                        )
                        is_empty = stripped_line.endswith('{};')
                        HeaderPostProcessor.record_class_definition(
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

            # remove useless thunks.
            if '// NOLINTEND' in line and (in_useless_thunk or in_static_variable):
                in_useless_thunk = False
                in_static_variable = False
                continue  # don't add this line.
            for token in RECORDED_THUNKS:
                if token in line:
                    in_useless_thunk = True
                    is_modified = True

                    # remove previous access specifier.
                    content = content[: content.rfind('public:')]
                    content = content[: content.rfind('\n')]  # for nested classes
            if not in_useless_thunk:
                content += line
        if is_modified:
            if args.fix_includes_for_member_variables and has_typed_storage:
                HeaderPostProcessor.add_pending_fix_includes_queue(
                    path_to_file, forward_declarations, member_variable_types
                )
            if args.fix_size_for_type_with_empty_template_class and has_typed_storage:
                HeaderPostProcessor.add_pending_fix_members_queue(
                    path_to_file, member_variable_types
                )
            with open(path_to_file, 'w', encoding='utf-8') as wfile:
                wfile.write(content)


def iterate(args: Options):
    assert os.path.isdir(args.base_dir)

    for root, dirs, files in os.walk(args.base_dir):
        for file in files:
            if CppUtil.is_header_file(file):
                path = os.path.join(root, file)
                # preprocessing: execution time is before each file.
                HeaderPreProcessor.process(path)
                # processing: executed immediately after preprocessing is completed.
                process(path, args)

    # post-processing: executed after all files have been processed.
    #                  during processing, files that require post-processing will be marked.
    HeaderPostProcessor.process()
