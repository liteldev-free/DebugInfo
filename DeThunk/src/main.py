import sys
import os


def remove_thunks(path_to_file: str):
    if not os.path.isfile(path_to_file):
        print('invalid file.')
        return

    with open(path_to_file, 'r', encoding='utf-8') as file:
        # states
        is_modified = False
        in_useless_thunk = False
        in_static_variable = False

        # tmp
        content = ''
        for line in file.readlines():
            # restore static member variable thunk:
            if '// static variables' in line:
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
                    if call_spec_pos == -1:
                        print(path_to_file)
                        assert False
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
            for token in [
                '// virtual function thunks',
                '// constructor thunks',
                '// vftables',
                '// destructor thunk',
            ]:
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


def iterate_headers(target_path: str):
    def is_header(path: str):
        return path.endswith('.h') or path.endswith('.hpp')

    if not os.path.isdir(target_path):
        print('invalid path.')
        return

    for root, dirs, files in os.walk(target_path):
        for file in files:
            if is_header(file):
                remove_thunks(os.path.join(root, file))


def main():
    if len(sys.argv) < 2:
        print('usage: main.py <PATH>')
        return

    iterate_headers(sys.argv[1])

    print('done.')


if __name__ == '__main__':
    main()
