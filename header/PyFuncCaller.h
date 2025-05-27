// PyFuncCaller.h
#ifndef PY_FUNC_CALLER_H
#define PY_FUNC_CALLER_H

#include "ConvertHelper.h" // 型変換ヘルパー
#include "PyInitializer.h" // Pythonインタプリタの初期化ヘルパー
using namespace convert_helper;

class PyFuncCaller {
public:
    PyFuncCaller(const std::string& moduleName, const std::string& functionName)
        : pModule_(nullptr), pFunc_(nullptr) {
        // Pythonが初期化されていることを確認
        if (!Py_IsInitialized()) {
            throw std::runtime_error("Python interpreter not initialized. Create a PyInitializer instance first.");
        }

        pModule_ = PyImport_ImportModule(moduleName.c_str());
        if (!pModule_) {
            PyErr_Print();
            throw std::runtime_error("Failed to import Python module: " + moduleName);
        }

        pFunc_ = PyObject_GetAttrString(pModule_, functionName.c_str());
        if (!pFunc_ || !PyCallable_Check(pFunc_)) {
            Py_XDECREF(pModule_); // モジュールはもう不要なので解放
            pModule_ = nullptr;
            PyErr_Print();
            throw std::runtime_error("Failed to find callable Python function: " + functionName + " in module " + moduleName);
        }
    }

    ~PyFuncCaller() {
        Py_XDECREF(pFunc_);
        Py_XDECREF(pModule_);
    }

    // コピーとムーブを禁止
    PyFuncCaller(const PyFuncCaller&) = delete;
    PyFuncCaller& operator=(const PyFuncCaller&) = delete;

    template<typename Ret, typename... Args>
    Ret call(Args&&... args) { // 右辺値参照で引数を完全転送
        if (!pFunc_) { // 通常コンストラクタでチェックされるが念のため
            throw std::runtime_error("Python function is not loaded.");
        }

        constexpr size_t numArgs = sizeof...(Args);
        PyObject* pArgsTuple = nullptr;

        if constexpr (numArgs > 0) {    
            // 引数をPyObject*に変換して一時的に保持
            std::array<PyObject*, numArgs> pyArgsObjects;
            size_t current_arg_idx = 0;
            bool conversion_success = true;

            // 各引数を変換
            ( ([&]{
                if (!conversion_success) return;
                PyObject* py_arg = convertToPyObject(std::forward<Args>(args)); // 完全転送
                if (!py_arg) {
                    // 変換失敗、既に変換したものを解放
                    for (size_t k = 0; k < current_arg_idx; ++k) {
                        Py_DECREF(pyArgsObjects[k]);
                    }
                    conversion_success = false;
                    PyErr_Print(); // convertToPyObjectがエラーを設定している可能性
                    return;
                }
                pyArgsObjects[current_arg_idx++] = py_arg; // 新しい参照を格納
            }()), ...);
            // C++11/14の場合は、手動でループするか、初期化子リストトリックを使う

            if (!conversion_success) {
                throw std::runtime_error("Failed to convert one or more C++ arguments to Python objects.");
            }

            pArgsTuple = PyTuple_New(numArgs);
            if (!pArgsTuple) {
                for (size_t k = 0; k < numArgs; ++k) Py_DECREF(pyArgsObjects[k]);
                PyErr_Print();
                throw std::runtime_error("Failed to create Python arguments tuple.");
            }

            for (size_t i = 0; i < numArgs; ++i) {
                // PyTuple_SetItem は pyArgsObjects[i] の参照を盗む
                if (PyTuple_SetItem(pArgsTuple, i, pyArgsObjects[i]) != 0) {
                    Py_DECREF(pArgsTuple); // タプル自体と、そこに含まれる要素を解放
                    // pyArgsObjects[i] 以降でまだタプルにセットされていないものを解放
                    for (size_t k = i; k < numArgs; ++k) { // SetItem に失敗した現在の要素も含む
                        Py_DECREF(pyArgsObjects[k]);
                    }
                    PyErr_Print();
                    throw std::runtime_error("Failed to set item in Python arguments tuple.");
                }
                // 成功した場合、pyArgsObjects[i] の所有権はタプルに移る
            }
        } else { // 引数なし
            pArgsTuple = PyTuple_New(0);
            if (!pArgsTuple) {
                PyErr_Print();
                throw std::runtime_error("Failed to create empty Python arguments tuple.");
            }
        }

        PyObject* pReturnValue = PyObject_CallObject(pFunc_, pArgsTuple);
        Py_DECREF(pArgsTuple); // 引数タプルは呼び出し後に不要

        if (!pReturnValue) {
            PyErr_Print();
            throw std::runtime_error("Python function call failed.");
        }

        // 戻り値の処理
        if constexpr (std::is_same_v<Ret, void>) {
            Py_DECREF(pReturnValue); // voidでも戻り値オブジェクトはDECREF
            return;
        } else {
            Ret result;
            try {
                result = convertFromPyObject<Ret>(pReturnValue);
            } catch (const std::exception& e) {
                Py_DECREF(pReturnValue); // 変換エラー時も忘れずに解放
                PyErr_Print(); // Python側のエラーも表示してみる
                throw std::runtime_error(std::string("Failed to convert Python return value: ") + e.what());
            }
            Py_DECREF(pReturnValue); // 変換後、戻り値オブジェクトは不要
            return result;
        }
    }

private:
    PyObject* pModule_;
    PyObject* pFunc_;
};

#endif // PY_FUNC_CALLER_H