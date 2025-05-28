// main.cpp
#include <iostream>
#include "../include/PyFuncCaller.h"

int main() {
    PFC::PyInitializer pyInit;   // Pythonインタプリタを初期化
    pyInit.addModuleDir("../python");   // Pythonモジュールのディレクトリを追加
    pyInit.addPythonDir("C:\\Miniconda3");    // PythonのDLLがあるディレクトリを追加

    std::cout << (pyInit.isInitialized() ? "python is initialized" : "python is not initialized") << std::endl;

    // --- python関数の呼び出し ---
    // ...
    PFC::PyFuncCaller add("example", "add");
    auto result = add.call<int>(1, 2);
    std::cout << "python: add(1, 2) = " << result << std::endl;
    
    return 0;
}

// main.cpp
