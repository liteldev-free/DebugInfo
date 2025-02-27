"""
String Utiltiy
"""

import re


def startswith_m(con: str, *args) -> bool:
    for arg in args:
        if con.startswith(arg):
            return True
    return False


def endswith_m(con: str, *args) -> bool:
    for arg in args:
        if con.endswith(arg):
            return True
    return False


def find_mb(con: str, *args) -> int:  # bounded
    r_pos = -1
    r_arg = None
    for arg in args:
        matched = re.search(rf'\b{re.escape(arg)}\b', con)
        if not matched:
            continue
        pos = matched.start()
        if pos != -1 and (r_pos == -1 or pos < r_pos):
            r_pos = pos
            r_arg = arg
    return r_pos, r_arg


def find_m(con: str, *args) -> int:
    r_pos = -1
    r_arg = None
    for arg in args:
        pos = con.find(arg)
        if pos != -1 and (r_pos == -1 or pos < r_pos):
            r_pos = pos
            r_arg = arg
    return r_pos, r_arg


def rfind_m(con: str, *args) -> int:
    r_pos = -1
    r_arg = None
    for arg in args:
        pos = con.rfind(arg)
        if pos > r_pos:
            r_pos = pos
            r_arg = arg
    return r_pos, r_arg


def flatten(con: str):
    # typed storage can be very complex, so we need to preprocess it.
    # TODO: find a better way.
    return re.sub(r'\s+', ' ', con).replace('< ', '<').replace(':: ', '::')
