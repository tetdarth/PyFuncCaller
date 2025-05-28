// PyFuncCaller.h
#pragma once

#include "ConvertHelper.h" // 型変換ヘルパー
#include <windows.h>
#include <filesystem>

namespace pfc {

// Python関数を呼び出すためのクラス
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
            Py_XDECREF(pModule_); // モジュールを開放
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
    Ret call(Args&&... args) {
        if (!pFunc_) {
            throw std::runtime_error("Python function is not loaded.");
        }

        constexpr size_t numArgs = sizeof...(Args);
        PyObject* pArgsTuple = nullptr;

        if constexpr (numArgs > 0) {    
            std::array<PyObject*, numArgs> pyArgsObjects;
            size_t current_arg_idx = 0;
            bool conversion_success = true;

            // 各引数を変換
            ( ([&]{
                if (!conversion_success) return;
                PyObject* py_arg = convertToPyObject(std::forward<Args>(args)); // 完全転送
                if (!py_arg) {
                    for (size_t k = 0; k < current_arg_idx; ++k) {
                        Py_DECREF(pyArgsObjects[k]);
                    }
                    conversion_success = false;
                    PyErr_Print();
                    return;
                }
                pyArgsObjects[current_arg_idx++] = py_arg; // 新しい参照を格納
            }()), ...);

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
                if (PyTuple_SetItem(pArgsTuple, i, pyArgsObjects[i]) != 0) {
                    Py_DECREF(pArgsTuple);
                    for (size_t k = i; k < numArgs; ++k) {
                        Py_DECREF(pyArgsObjects[k]);
                    }
                    PyErr_Print();
                    throw std::runtime_error("Failed to set item in Python arguments tuple.");
                }
            }
        } else { // 引数なし
            pArgsTuple = PyTuple_New(0);
            if (!pArgsTuple) {
                PyErr_Print();
                throw std::runtime_error("Failed to create empty Python arguments tuple.");
            }
        }

        PyObject* pReturnValue = PyObject_CallObject(pFunc_, pArgsTuple);
        Py_DECREF(pArgsTuple); // 引数タプルは呼び出し後にGC

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
                Py_DECREF(pReturnValue); // 変換エラー時もDECREF
                PyErr_Print();
                throw std::runtime_error(std::string("Failed to convert Python return value: ") + e.what());
            }
            Py_DECREF(pReturnValue); // 変換後、戻り値オブジェクトはDECREF
            return result;
        }
    }

private:
    PyObject* pModule_;
    PyObject* pFunc_;
};

// Pythonインタプリタの初期化と終了を管理するクラス
class PyInitializer {
public:
    PyInitializer() {
        if (Py_IsInitialized()) return; // 既に初期化されている場合は何もしない
        Py_Initialize();    // Pythonインタプリタを初期化
    }

    ~PyInitializer() {
        if (Py_IsInitialized()) {
            Py_FinalizeEx();    // Pythonインタプリタを終了
        }
    }

    // コピーとムーブを禁止
    PyInitializer(const PyInitializer&) = delete;
    PyInitializer& operator=(const PyInitializer&) = delete;
    PyInitializer(PyInitializer&&) = delete;
    PyInitializer& operator=(PyInitializer&&) = delete;

    // Moduleのディレクトリを追加する
    void addModuleDir(const std::string& dir) const {
        if (!std::filesystem::exists(dir)) {
            throw std::runtime_error("Module path does not exist: " + dir);
        }
        std::string code = "import sys; sys.path.append('" + dir + "')";
        PyRun_SimpleString(code.c_str());
    }

    // Pythonのpython3x.dllとzlib.dllを取得するためのディレクトリ
    void addPythonDir(const std::string& dir) const {
        if (!std::filesystem::exists(dir)) {
            throw std::runtime_error("Module path does not exist: " + dir);
        }
        SetDllDirectoryA(dir.c_str());
    }

    // インタプリタが初期化されているかを返す関数
    bool isInitialized() const {
        return Py_IsInitialized() != 0;
    }
};

}

// PyFuncCaller.h