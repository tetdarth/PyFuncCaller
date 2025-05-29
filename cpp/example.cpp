// main.cpp
#include <iostream>

#include "../include/PyFuncCaller.h"

int main() {
    pfc::PyInitializer pyInit;   // Pythonインタプリタを初期化
    pyInit.addModuleDir("../python");   // Pythonモジュールのディレクトリを追加
    pyInit.addPythonDir("C:\\Miniconda3");    // PythonのDLLがあるディレクトリを追加

    std::cout << (pyInit.isInitialized() ? "python is initialized" : "python is not initialized") << std::endl;

    // --- 引数がint2つ、返り値がint ---
    pfc::PyFuncCaller add("example", "add");
    auto result = add.call<int>(1, 2);
    std::cout << "add(1, 2) = " << result << std::endl;

    // --- 返り値がvoid ---
    pfc::PyFuncCaller isBool("example", "isBool");
    isBool.call<void>(false);

    // --- 引数がint2つ、返り値がbool ---
    pfc::PyFuncCaller isDivisor("example", "isDivisor");
    auto isDivisorResult = isDivisor.call<bool>(5, 15); // 5は15の約数かどうかを確認
    std::cout << "isDivisor(5, 15) = " << (isDivisorResult ? "true" : "false") << std::endl;

    // --- 返り値がstd::vector<int or double> ---
    pfc::PyFuncCaller getList("example", "getList");
    auto listResult1 = getList.call<std::vector<int>>(0, 10);
    for (const auto& item : listResult1) std::cout << item << ", ";   // 出力: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    std::cout << std::endl;

    auto listResult2 = getList.call<std::vector<double>>(0, 1, 0.2);
    for (const auto& item : listResult2) std::cout << item << ", ";   // 出力: 0, 0.2, 0.4, 0.6, 0.8,
    std::cout << std::endl;

    // --- 返り値が複数ある場合 ---
    pfc::PyFuncCaller getTuple("example", "getTuple");
    auto tupleResult = getTuple.call<std::tuple<int, std::string>>(42, "Hello");
    std::cout << "getTuple(42, 'Hello') = (" 
              << std::get<0>(tupleResult) << ", " 
              << std::get<1>(tupleResult) << ")" << std::endl; // getTuple(42, 'Hello') = (42, Hello)

    // --- 複数の行列積を計算する関数 ---
    using matrix = std::vector<std::vector<double>>;
    pfc::PyFuncCaller matrixMultiply("example", "matMultiply");

    matrix A = {{1, 2}, {3, 4}};
    matrix B = {{5, 6}, {7, 8}};
    auto matrixs = matrixMultiply.call<std::tuple<matrix, matrix, matrix, matrix>>(A, B);
    auto printMatrix = [](const matrix& m, const std::string& name = "") {
        std::cout << name << ":\n";
        for (const auto& row : m) {
            for (const auto& val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "---" << std::endl;
    };
    printMatrix(std::get<0>(matrixs), "A * A");
    printMatrix(std::get<1>(matrixs), "A * B");
    printMatrix(std::get<2>(matrixs), "B * A");
    printMatrix(std::get<3>(matrixs), "B * B");

    return 0;
}

// main.cpp
