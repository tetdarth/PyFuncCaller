// main.cpp
#include <iostream>

#include "../include/PyFuncCaller.h"

int main() {
    pfc::PyInitializer pyInit;   // Pythonインタプリタを初期化
    pyInit.addModuleDir("../python");   // Pythonモジュールのディレクトリを追加
    pyInit.addPythonDir("C:\\Miniconda3");    // PythonのDLLがあるディレクトリを追加

    std::cout << (pyInit.isInitialized() ? "python is initialized" : "python is not initialized") << std::endl;

    // --- python関数の呼び出し ---
    // ...
    pfc::PyFuncCaller add("example", "add");
    auto result = add.call<int>(1, 2);
    std::cout << "add(1, 2) = " << result << std::endl;

    pfc::PyFuncCaller isBool("example", "isBool");
    isBool.call<void>(false);

    pfc::PyFuncCaller isDivisor("example", "isDivisor");
    auto isDivisorResult = isDivisor.call<bool>(5, 15); // 5は15の約数かどうかを確認
    std::cout << "isDivisor(5, 15) = " << (isDivisorResult ? "true" : "false") << std::endl;
    
    return 0;
}

// main.cpp
