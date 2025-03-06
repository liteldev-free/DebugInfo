import os
import sys
import shutil


def main():
    assert len(sys.argv) == 4, 'invalid options.'

    # without 'mc' suffix.
    ll_mc_base = sys.argv[1]
    lf_mc_base = sys.argv[2]

    empty_list = sys.argv[3]

    assert ll_mc_base.endswith('/src/')
    assert lf_mc_base.endswith('/src/')

    assert os.path.isdir(ll_mc_base)
    assert os.path.isdir(lf_mc_base)
    assert os.path.isfile(empty_list)

    with open(empty_list) as file:
        content = file.read()
        for empty_file in content.split('\n'):
            if len(empty_file) > 0:
                shutil.copyfile(ll_mc_base + empty_file, lf_mc_base + empty_file)


if __name__ == '__main__':
    main()
