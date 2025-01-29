"""
String Utiltiy
"""


def startswith_m(con: str, *args) -> bool:
    for arg in args:
        if con.startswith(arg):
            return True
    return False


def find_m(con: str, *args) -> int:
    for arg in args:
        pos = con.find(arg)
        if pos != -1:
            return pos, arg
    return -1
