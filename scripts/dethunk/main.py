import argparse

import header_processor as HeaderProcessor


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

    parser.add_argument('--preset-extract-names', action='store_true')
    parser.add_argument('--preset-extract-types', action='store_true')

    args = parser.parse_args()

    HeaderProcessor.iterate(HeaderProcessor.Options(args))

    print('done.')


if __name__ == '__main__':
    main()
