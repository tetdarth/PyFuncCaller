#include "PyFuncCaller.h"
#include <iostream>
#include <random>

// 2次元ベクタを乱数で初期化
std::vector<std::vector<double>> random_matrix(size_t rows, size_t cols, double min = 0.0, double max = 1.0) {
    std::vector<std::vector<double>> mat(rows, std::vector<double>(cols));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(min, max);
    for (auto& row : mat)
        for (auto& v : row)
            v = dist(gen);
    return mat;
}

int main() {
    printf("starting C++ application...");
    try {
        PyInitializer pyInit; // Pythonインタプリタを初期化
        pyInit.addModulePath("../python/"); // module(~.py)があるディレクトリを指定

        PyFuncCaller greeter("my_module", "greet");
        int length = greeter.call<int>("World", 3); // 戻り値: int, 引数: const char*, int
        std::cout << "C++: Length of greeting returned by Python: " << length << std::endl;

        PyFuncCaller adder("my_module", "add");
        double sum = adder.call<double>(5.5, 2.2); // 戻り値: double, 引数: double, double
        std::cout << "C++: Sum from Python: " << sum << std::endl;

        int sum_int = adder.call<int>(10, 20);
        std::cout << "C++: Integer sum from Python: " << sum_int << std::endl;
        
        PyFuncCaller no_op("my_module", "no_args_no_return");
        no_op.call<void>(); // 戻り値なし、引数なし
        std::cout << "C++: Called Python function with no args/return." << std::endl;

        // 行列積
        std::vector<std::vector<int>> x = {
            {2, 4, 3},
            {3, 1, 0},
            {2, 2, 5}
        };
        std::vector<std::vector<int>> y = {
            {1, 0, 3},
            {4, 3, 1},
            {2, 0, 2}
        };
        PyFuncCaller dot("my_module", "dot_py");
        std::vector<std::vector<int>> result = dot.call<std::vector<std::vector<int>>>(x, y);

        for (const auto& i : result) {
            std::cout << "[";
            for (const auto& j : i) {
                std::cout << j << ", ";
            }
            std::cout << "]" << std::endl;
        }

        // 巨大逆行列演算
        PyFuncCaller inv_mat("my_module", "inverse_matrix");
        inv_mat.call<void>(random_matrix(1000, 1000));

        int tmp;
        std::cin >> tmp;


    } catch (const std::exception& e) {
        std::cerr << "C++ Exception: " << e.what() << std::endl;
        return 1;
    }
    
    PyFinalizer pyFinalizer; // ここで自動的にPy_FinalizeEx()が呼ばれる

    return 0;
}