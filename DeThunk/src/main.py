import os
import argparse


class ProcessorOptions:
    base_dir = str()

    # functions
    remove_constructor_thunk = bool()
    remove_destructor_thunk = bool()
    remove_virtual_function_thunk = bool()

    # variables
    remove_virtual_table_pointer_thunk = bool()
    restore_static_variable = bool()

    def __init__(self, args):
        self.base_dir = args.path
        self.remove_constructor_thunk = args.remove_constructor_thunk
        self.remove_destructor_thunk = args.remove_destructor_thunk
        self.remove_virtual_function_thunk = args.remove_virtual_function_thunk
        self.remove_virtual_table_pointer_thunk = args.remove_virtual_table_pointer_thunk
        self.restore_static_variable = args.restore_static_variable

        if args.all:
            self.set_all(True)

    def set_function(self, opt: bool):
        self.remove_constructor_thunk = opt
        self.remove_destructor_thunk = opt
        self.remove_virtual_function_thunk = opt

    def set_variable(self, opt: bool):
        self.remove_virtual_table_pointer_thunk = opt
        self.restore_static_variable = opt

    def set_all(self, opt: bool):
        self.set_function(opt)
        self.set_variable(opt)


def remove_thunks(path_to_file: str, args: ProcessorOptions):
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

        # tmp
        content = ''
        for line in file.readlines():
            # restore static member variable thunk:
            if args.restore_static_variable and '// static variables' in line:
                in_static_variable = True
                is_modified = True
            if in_static_variable:
                tmpline = line.strip()
                if tmpline.endswith(';'):
                    if not tmpline.startswith('MCAPI'):  # declaration may not be on one line
                        begin_pos = content.rfind('MCAPI')
                        tmpline = content[begin_pos:] + tmpline
                        content = content[:begin_pos]
                        tmpline = tmpline.strip()

                    # remove parameter list (convert to static variable)
                    call_spec_pos = tmpline.rfind('()')
                    assert call_spec_pos != -1

                    tmpline = tmpline[:call_spec_pos] + tmpline[call_spec_pos + 2 :]

                    # remove reference
                    refsym_pos = tmpline.rfind('&')  # T&
                    tlpsym_pos = tmpline.rfind('>')  # ::std::add_lvalue_reference_t<T>
                    assert refsym_pos != -1 or tlpsym_pos != -1
                    if tlpsym_pos == -1 or refsym_pos > tlpsym_pos:
                        tmpline = tmpline[:refsym_pos] + tmpline[refsym_pos + 1 :]
                    elif refsym_pos == -1 or tlpsym_pos > refsym_pos:
                        # C-style arrays must have '[]' written after the variable name
                        tmpline = tmpline[:tlpsym_pos] + '>' + tmpline[tlpsym_pos:]
                        tmpline = tmpline.replace(
                            '::std::add_lvalue_reference_t<',
                            '::std::remove_reference_t<::std::add_lvalue_reference_t<',
                        )

                    content += f'\t{tmpline}\n'
                    continue

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
                remove_thunks(os.path.join(root, file), args)


def main():
    parser = argparse.ArgumentParser('dethunk')

    parser.add_argument('path', help='Path to header project.')

    parser.add_argument('--remove-constructor-thunk', action='store_true')
    parser.add_argument('--remove-destructor-thunk', action='store_true')
    parser.add_argument('--remove-virtual-table-pointer-thunk', action='store_true')
    parser.add_argument('--remove-virtual-function-thunk', action='store_true')
    parser.add_argument('--restore-static-variable', action='store_true')

    parser.add_argument('--all', action='store_true', help='Apply all remove/restore options.')

    args = parser.parse_args()

    iterate_headers(ProcessorOptions(args))

    print('done.')


if __name__ == '__main__':
    main()
