// PyInitializer.h
#ifndef PY_INITIALIZER_H
#define PY_INITIALIZER_H

#include <windows.h>
#include <Python.h>
#include <filesystem>

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

    // Moduleのパスを追加する
    void addModulePath(const std::string& path) {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Module path does not exist: " + path);
        }
        std::string code = "import sys; sys.path.append('" + path + "')";
        PyRun_SimpleString(code.c_str());
    }

    // Pythonのpython3x.dllとzlib.dllを取得するためのパス
    void addPythonPath(const std::string& path) {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Module path does not exist: " + path);
        }
        SetDllDirectoryA(path.c_str());
    }
};

// アプリケーション終了時にPythonインタプリタを明示的に終了させるための専用クラス
class PyFinalizer {
public:
    ~PyFinalizer() {
        if (Py_IsInitialized()) {
            Py_FinalizeEx();
        }
    }
};

#endif // PY_INITIALIZER_H