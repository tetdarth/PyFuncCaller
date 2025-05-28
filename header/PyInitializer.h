// PyInitializer.h
#pragma once

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

    // Moduleのディレクトリを追加する
    void addModuleDir(const std::string& dir) {
        if (!std::filesystem::exists(dir)) {
            throw std::runtime_error("Module path does not exist: " + dir);
        }
        std::string code = "import sys; sys.path.append('" + dir + "')";
        PyRun_SimpleString(code.c_str());
    }

    // Pythonのpython3x.dllとzlib.dllを取得するためのディレクトリ
    void addPythonDir(const std::string& dir) {
        if (!std::filesystem::exists(dir)) {
            throw std::runtime_error("Module path does not exist: " + dir);
        }
        SetDllDirectoryA(dir.c_str());
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
