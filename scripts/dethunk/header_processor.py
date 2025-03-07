import os
import re

import header_preprocessor as HeaderPreProcessor

from options import Options

import util.cpp_language as CppUtil
import util.string as StrUtil


def try_translate_forward_declaration(
    decl: str, typeset: list
) -> HeaderPreProcessor.ClassDefine | None:
    find_decl = CppUtil.find_class_forward_declaration(decl)
    assert find_decl, f'decl = {decl}'

    namespace = find_decl.namespace_decl
    clazz = find_decl.class_decl

    if not CppUtil.is_full_type_required_for_typeset(namespace, clazz, typeset):
        return None

    return HeaderPreProcessor.query_class_record_strict(namespace, clazz)


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
        in_invalid_default_ctors = False
        has_typed_storage = False
        forward_declarations = []
        member_variable_types = []
        current_unions = []
        current_classes = []

        ll_typed_regex = re.compile(r'TypedStorage<(\d+), (\d+), (.*?)> (\w+);')
        ll_untyped_regex = re.compile(r'UntypedStorage<(\d+), (\d+)> (\w+);')

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

                if args.fix_msvc_c2734 and stripped_line.find('static ') == -1:
                    stripped_line = 'extern ' + stripped_line

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
                    matched = ll_typed_regex.search(StrUtil.flatten(stripped_line))
                    assert matched and matched.lastindex == 4, (
                        f'in {path_to_file}, line="{stripped_line}"'
                    )

                    align = int(matched[1])
                    size = int(matched[2])
                    type_name = matched[3]
                    var_name = matched[4]

                    if (
                        not StrUtil.endswith_m(type_name, '&', '*')
                        and args.fix_size_for_type_with_empty_template_class
                    ):
                        for empty_class in HeaderPreProcessor.empty_class_all_names:
                            if empty_class in type_name or (
                                args.erase_extra_invalid_types
                                and (
                                    args.should_erase_type(type_name)
                                    or (
                                        args.add_trivial_dynamic_initializer
                                        and args.should_erase_type_dyninit(type_name)
                                    )
                                )
                            ):
                                # print(f'erased: {type_name}')
                                content += (
                                    f'\talignas({align}) std::byte {var_name}_TYPEERASED[{size}];\n'
                                )
                                in_member_variable = False
                                break
                        if not in_member_variable:
                            continue

                    if args.add_trivial_dynamic_initializer:
                        type_name = type_name.replace(' const', '')

                    member_variable_types.append(type_name)

                    security_check = ''
                    if args.add_sizeof_alignof_static_assertions and not current_unions:
                        security_check += f'\tstatic_assert(sizeof({var_name}) == {size});\n'
                        # TODO: ensure alignment requirements.
                        # security_check += (
                        #     f'\tstatic_assert(alignof(decltype({var_name})) == {align});\n'
                        # )

                    if type_name.endswith(']'):  # is c-style array
                        array_defs = type_name[type_name.find('[') : type_name.rfind(']') + 1]
                        type_name = type_name[: type_name.find('[')]
                        var_name = var_name + array_defs

                    if type_name.endswith('&'):  # is reference type
                        type_name = type_name[:-1] + '*'

                    ptr_id_pos, ptr_id = StrUtil.find_m(type_name, '(*)', '::*)')
                    if -1 != ptr_id_pos and not CppUtil.find_template_name(
                        type_name, ptr_id, disable_regex_word_bound=True
                    ):  # is c-style function ptr or member ptr
                        # it's a trick, make sure `ptr` endswith '*)'
                        ptr_id_len = len(ptr_id) - 1
                        type_name = (
                            type_name[: ptr_id_pos + ptr_id_len]
                            + var_name
                            + type_name[ptr_id_pos + ptr_id_len :]
                        )
                        var_name = ''

                    content += f'\t{type_name} {var_name};\n{security_check}'

                    in_member_variable = False
                    continue

                # ::ll::UntypedStorage<Alignment, Size>  mVar;
                if 'UntypedStorage' in stripped_line:
                    matched = ll_untyped_regex.search(StrUtil.flatten(stripped_line))
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

            union_keyword_pos = line.find('union ')
            if not in_forward_declaration_list and union_keyword_pos != -1:
                current_unions.append(union_keyword_pos)
            if current_unions and line.startswith(' ' * current_unions[-1] + '};'):
                current_unions.pop()

            if not in_forward_declaration_list:
                founded_cl = CppUtil.find_class_definition(line)
                if founded_cl:
                    class_keyword_pos, _ = StrUtil.find_mb(line, 'class ', 'struct ', 'union ')
                    assert class_keyword_pos != -1, f"path = {path_to_file}, line = '{line}'"

                    if not stripped_line.endswith('{};'):
                        current_classes.append([class_keyword_pos, founded_cl])

            if current_classes and line.startswith(' ' * (current_classes[-1][0]) + '};'):
                current_classes.pop()

            # fix forward declarations
            if stripped_line.startswith('// auto generated forward declare list'):
                in_forward_declaration_list = True
            if in_forward_declaration_list and StrUtil.startswith_m(
                stripped_line, 'class ', 'struct ', 'union ', 'namespace '
            ):
                forward_declarations.append(stripped_line)
            if stripped_line.startswith('// clang-format on') and in_forward_declaration_list:
                in_forward_declaration_list = False

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

            if args.add_trivial_dynamic_initializer:
                if in_invalid_default_ctors:
                    if 'operator=' in line or 'const&' in line or '()' in line:
                        content += '\t// ' + stripped_line + '\n'
                        continue
                    else:
                        in_invalid_default_ctors = False
                if '// prevent constructor by default' in line:
                    in_invalid_default_ctors = True

                if current_classes and stripped_line.endswith(';'):
                    begin_pos = None
                    if not stripped_line.startswith('MCAPI'):
                        begin_pos = content.rfind('MCAPI')
                        check_pos = content.rfind(';')
                        if begin_pos > check_pos:
                            stripped_line = content[begin_pos:] + stripped_line
                            stripped_line = StrUtil.flatten(stripped_line)
                    class_name = current_classes[-1][1]
                    if (
                        f' {class_name}()' not in stripped_line
                        and f' {class_name}(' in stripped_line
                    ):
                        if begin_pos:
                            content = content[:begin_pos]
                        content += '\t// ' + stripped_line + '\n'
                        continue

            if not in_useless_thunk:
                content += line
        if args.fix_includes_for_member_variables and has_typed_storage:
            for decl in forward_declarations:
                if args.add_trivial_dynamic_initializer and args.should_ignore_forward_decl(decl):
                    continue
                class_define = try_translate_forward_declaration(decl, member_variable_types)
                if class_define:
                    is_modified = True
                    content = content.replace(decl, f'#include "{class_define.rpath}"\n')
                    continue
        if args.add_trivial_dynamic_initializer:
            path = path_to_file[path_to_file.find('src/') + 4 :]
            if path in HeaderPreProcessor.defined_classes_path:
                for ns, classes in HeaderPreProcessor.defined_classes_path[path].items():
                    for cl in classes:
                        if HeaderPreProcessor.defined_classes[ns][cl].is_template:
                            continue
                        var_name = 'dummy__'
                        if ns:
                            var_name += ns.replace('::', '_') + '_'
                        var_name += cl.replace('::', '_')
                        full_name = ns
                        if ns:
                            full_name += '::'
                        full_name += cl
                        if args.should_ignore_generate_dynamic_initializer(full_name):
                            continue
                        is_modified = True
                        content += f'\ninline {full_name} {var_name};'
            # rm pure virtual funcs.
            is_modified = True
            content = (
                content.replace(' = 0;', ';')
                .replace('::std::reference_wrapper<', '::x_std::reference_wrapper<')
                .replace('::Bedrock::NotNullNonOwnerPtr<', '::x_Bedrock::NotNullNonOwnerPtr<')
                .replace('::gsl::not_null<', '::x_gsl::not_null<')
                .replace('::leveldb::EnvWrapper', '::x_leveldb::EnvWrapper')
            )
        if is_modified:
            with open(path_to_file, 'w', encoding='utf-8') as wfile:
                wfile.write(content)


def iterate(args: Options):
    assert os.path.isdir(args.base_dir)

    def _traverse_header(path_to_dir: str, fun):
        for root, dirs, files in os.walk(path_to_dir):
            for file in files:
                if CppUtil.is_header_file(file):
                    path = os.path.join(root, file)
                    fun(path)

    def _preprocess(path_to_file):
        HeaderPreProcessor.process(path_to_file)

    def _process(path_to_file):
        process(path_to_file, args)

    _traverse_header(args.base_dir, _preprocess)
    _traverse_header(args.base_dir, _process)
