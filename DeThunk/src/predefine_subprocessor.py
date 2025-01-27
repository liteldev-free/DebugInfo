"""
Preprocessor for _HeaderOutputPredefine.h
"""


def process(path_to_file: str):
    with open(path_to_file, 'r', encoding='utf-8') as file:
        content = file.read()

        content += '\n#include <winsock2.h>'

        with open(path_to_file, 'w', encoding='utf-8') as wfile:
            wfile.write(content)
