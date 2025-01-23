import os
import argparse
import re


class ProcessorOptions:
    base_dir = str()

    # functions
    remove_constructor_thunk = bool()
    remove_destructor_thunk = bool()
    remove_virtual_function_thunk = bool()

    # variables
    remove_virtual_table_pointer_thunk = bool()
    restore_static_variable = bool()
    restore_member_variable = bool()

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


def process_headers(path_to_file: str, args: ProcessorOptions):
    assert os.path.isfile(path_to_file)

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

        ll_typed_regex = re.compile(r'TypedStorage<(\d+), (\d+), (.*?)> (\w+);')
        ll_untyped_regex = re.compile(r'UntypedStorage<(\d+), (\d+)> (\w+);')

        def regex_preprocess_name(con: str):
            # typed storage can be very complex, so we need to preprocess it.
            # TODO: find a better way.
            return re.sub(r'\s+', ' ', con).replace('< ', '<')

        # tmp
        content = ''
        for line in file.readlines():
            # restore static member variable thunk:
            if args.restore_static_variable and '// static variables' in line:
                in_static_variable = True
                is_modified = True
            if in_static_variable:
                this_line = line.strip()
                if this_line.endswith(';'):
                    if not this_line.startswith('MCAPI'):  # declaration may not be on one line
                        begin_pos = content.rfind('MCAPI')
                        this_line = content[begin_pos:] + this_line
                        content = content[:begin_pos]
                        this_line = this_line.strip()

                    # remove parameter list (convert to static variable)
                    call_spec_pos = this_line.rfind('()')
                    assert call_spec_pos != -1

                    this_line = this_line[:call_spec_pos] + this_line[call_spec_pos + 2 :]

                    # remove reference
                    refsym_pos = this_line.rfind('&')  # T&
                    tlpsym_pos = this_line.rfind('>')  # ::std::add_lvalue_reference_t<T>
                    assert refsym_pos != -1 or tlpsym_pos != -1, f'in {path_to_file}'
                    if tlpsym_pos == -1 or refsym_pos > tlpsym_pos:
                        this_line = this_line[:refsym_pos] + this_line[refsym_pos + 1 :]
                    elif refsym_pos == -1 or tlpsym_pos > refsym_pos:
                        # C-style arrays must have '[]' written after the variable name
                        this_line = this_line[:tlpsym_pos] + '>' + this_line[tlpsym_pos:]
                        this_line = this_line.replace(
                            '::std::add_lvalue_reference_t<',
                            '::std::remove_reference_t<::std::add_lvalue_reference_t<',
                        )

                    content += f'\t{this_line}\n'
                    continue

            if args.restore_member_variable and '::ll::' in line:  # union { ... };
                in_member_variable = True
                is_modified = True
            if in_member_variable:
                # ::ll::TypedStorage<Alignment, Size, T> mVar;
                # ::ll::UntypedStorage<Alignment, Size>  mVar;

                this_line = line.strip()
                if this_line.endswith(';'):
                    if not this_line.startswith('::ll::'):
                        begin_pos = content.rfind('::ll::')
                        this_line = content[begin_pos:] + this_line
                        content = content[:begin_pos]
                        this_line = this_line.strip()

                    if 'TypedStorage' in this_line:
                        matched = ll_typed_regex.search(regex_preprocess_name(this_line))
                        assert matched and matched.lastindex == 4, (
                            f'in {path_to_file}, line="{this_line}"'
                        )

                        align = matched[1]  # unused.
                        size = matched[2]  # unused.
                        type_name = matched[3]
                        var_name = matched[4]

                        content += f'\t{type_name} {var_name};\n'

                        in_member_variable = False
                        continue

                    if 'UntypedStorage' in this_line:
                        matched = ll_untyped_regex.search(regex_preprocess_name(this_line))
                        assert matched and matched.lastindex == 3, (
                            f'in {path_to_file}, line="{this_line}"'
                        )

                        align = matched[1]
                        size = matched[2]
                        var_name = matched[3]

                        content += f'\talignas({align}) std::byte {var_name}[{size}];\n'

                        in_member_variable = False
                        continue

                    assert False, 'unreachable'

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
            with open(path_to_file, 'w', encoding='utf-8') as wfile:
                wfile.write(content)


def iterate_headers(args: ProcessorOptions):
    assert os.path.isdir(args.base_dir)

    def is_cxx_header(path: str):
        return path.endswith('.h') or path.endswith('.hpp')

    for root, dirs, files in os.walk(args.base_dir):
        for file in files:
            if is_cxx_header(file):
                process_headers(os.path.join(root, file), args)


def main():
    parser = argparse.ArgumentParser('dethunk')

    parser.add_argument('path', help='Path to header project.')

    parser.add_argument('--remove-constructor-thunk', action='store_true')
    parser.add_argument('--remove-destructor-thunk', action='store_true')
    parser.add_argument('--remove-virtual-table-pointer-thunk', action='store_true')
    parser.add_argument('--remove-virtual-function-thunk', action='store_true')
    parser.add_argument('--restore-static-variable', action='store_true')
    parser.add_argument('--restore-member-variable', action='store_true')

    parser.add_argument('--all', action='store_true', help='Apply all remove/restore options.')

    args = parser.parse_args()

    iterate_headers(ProcessorOptions(args))

    print('done.')


if __name__ == '__main__':
    main()
