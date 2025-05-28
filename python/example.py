def add(x, y):
    return x + y

def isBool(b):
    print("PyTrue") if b else print("PyFalse")
    
def isDivisor(x, y):
    if x > y:
        return False
    return True if (y%x == 0) else False