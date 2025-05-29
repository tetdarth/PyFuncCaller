import numpy as np

def add(x, y):
    return x + y

def isBool(b):
    print("PyTrue") if b else print("PyFalse")
    
def isDivisor(x, y):
    if x > y:
        return False
    return True if (y%x == 0) else False

def getList(start, end, interval=1) -> list:
    return np.arange(start, end, interval).tolist()

def getTuple(n: int, string: str) -> tuple[int, str]:
    return n, string

def matMultiply(x : list, y : list) -> tuple[list, list, list, list]:
    x = np.array(x)
    y = np.array(y)
    return (x*x).tolist(), (x*y).tolist(), (y*x).tolist(), (y*y).tolist()
