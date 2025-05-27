# my_module.py
import numpy as np

def greet(name, times):
    greeting = f"Hello, {name}! " * times
    print(f"Python says: {greeting}")
    return len(greeting)

def add(a, b):
    return a + b

def no_args_no_return():
    print("Python: function with no args and no return called.")
    
def dot_py(x : list, y : list):
    x, y = np.array(x), np.array(y)
    return np.dot(x, y).tolist()

def inverse_matrix(x: list):
    x = np.array(x)
    print(np.linalg.inv(x))