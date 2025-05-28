// convertHelper.h
#pragma once

#include <Python.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <array>
#include <tuple>
#include <utility>
#include <climits>
#include <type_traits>

namespace PFC {

// --- is_vector型トレイト ---
template<typename T>
struct is_vector : std::false_type {};
template<typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

// --- is_tuple型トレイト ---
template<typename T>
struct is_tuple : std::false_type {};
template<typename... Ts>
struct is_tuple<std::tuple<Ts...>> : std::true_type {};

// ベーステンプレート
template<typename T>
T convertFromPyObject(PyObject* obj);

// tupleFromPyTupleImpl
template <typename Tuple, std::size_t... I>
Tuple tupleFromPyTupleImpl(PyObject* obj, std::index_sequence<I...>) {
    if (!PyTuple_Check(obj)) {
        throw std::runtime_error("Python object is not a tuple.");
    }
    constexpr std::size_t N = sizeof...(I);
    if (PyTuple_Size(obj) != N) {
        throw std::runtime_error("Tuple size mismatch.");
    }
    // std::make_tupleで初期化
    return std::make_tuple(convertFromPyObject<std::tuple_element_t<I, Tuple>>(PyTuple_GetItem(obj, I))...);
}


// --- C++ to PyObject* ---
template<typename T>
PyObject* convertToPyObject(T value); // ベーステンプレート

template<>
PyObject* convertToPyObject<long>(long value) {
    return PyLong_FromLong(value);
}

template<>
PyObject* convertToPyObject<int>(int value) {
    return PyLong_FromLong(static_cast<long>(value));
}

template<>
PyObject* convertToPyObject<double>(double value) {
    return PyFloat_FromDouble(value);
}

template<>
PyObject* convertToPyObject<const char*>(const char* value) {
    return PyUnicode_FromString(value);
}

template<>
PyObject* convertToPyObject<std::string>(std::string value) {
    return PyUnicode_FromString(value.c_str());
}

// n次元vectorをPyObject*（Pythonリスト）に再帰的に変換
template<typename T>
PyObject* convertToPyObject(const std::vector<T>& vec) {
    PyObject* pyList = PyList_New(vec.size());
    if (!pyList) throw std::runtime_error("Failed to create Python list.");
    for (size_t i = 0; i < vec.size(); ++i) {
        PyObject* item = convertToPyObject(vec[i]);
        if (!item) {
            Py_DECREF(pyList);
            throw std::runtime_error("Failed to convert vector element to PyObject.");
        }
        PyList_SET_ITEM(pyList, i, item); // Steals reference
    }
    return pyList;
}


// --- PyObject* to C++ ---

// メインテンプレート
template<typename T>
T convertFromPyObject(PyObject* obj) {
    if constexpr (std::is_same_v<T, int>) {
        long val = convertFromPyObject<long>(obj);
        if (val > INT_MAX || val < INT_MIN) {
            throw std::overflow_error("Python long value out of C++ int range.");
        }
        return static_cast<int>(val);
    } else if constexpr (std::is_same_v<T, long>) {
        if (!obj || !PyLong_Check(obj)) {
            if (PyErr_Occurred()) PyErr_Print();
            throw std::runtime_error("Cannot convert Python object to long.");
        }
        long val = PyLong_AsLong(obj);
        if (val == -1 && PyErr_Occurred()) {
            PyErr_Print();
            throw std::runtime_error("Error converting Python long to C++ long (overflow or type error).");
        }
        return val;
    } else if constexpr (std::is_same_v<T, double>) {
        if (!obj || !PyFloat_Check(obj)) {
            if (!PyLong_Check(obj)) {
                if (PyErr_Occurred()) PyErr_Print();
                throw std::runtime_error("Cannot convert Python object to double.");
            }
        }
        double val = PyFloat_AsDouble(obj);
        if (val == -1.0 && PyErr_Occurred()) {
            PyErr_Print();
            throw std::runtime_error("Error converting Python float/long to C++ double.");
        }
        return val;
    } else if constexpr (std::is_same_v<T, std::string>) {
        if (!obj) {
            throw std::runtime_error("Cannot convert null Python object to string.");
        }
        if (PyUnicode_Check(obj)) {
            PyObject* utf8_bytes = PyUnicode_AsUTF8String(obj);
            if (!utf8_bytes) {
                PyErr_Print();
                throw std::runtime_error("Failed to convert Python Unicode to UTF-8.");
            }
            std::string result = PyBytes_AsString(utf8_bytes);
            Py_DECREF(utf8_bytes);
            return result;
        } else if (PyBytes_Check(obj)) {
            return PyBytes_AsString(obj);
        }
        PyObject* str_obj = PyObject_Str(obj);
        if (!str_obj) {
            PyErr_Print();
            throw std::runtime_error("Cannot convert Python object to string representation.");
        }
        PyObject* utf8_bytes = PyUnicode_AsUTF8String(str_obj);
        Py_DECREF(str_obj);
        if (!utf8_bytes) {
            PyErr_Print();
            throw std::runtime_error("Failed to convert Python object's string representation to UTF-8.");
        }
        std::string result = PyBytes_AsString(utf8_bytes);
        Py_DECREF(utf8_bytes);
        return result;
    } else if constexpr (std::is_same_v<T, void>) {
        return;
    } else if constexpr (is_vector<T>::value) {
        using ElemType = typename T::value_type;
        if (!obj || !PyList_Check(obj)) {
            throw std::runtime_error("Python object is not a list.");
        }
        Py_ssize_t size = PyList_Size(obj);
        T result;
        result.reserve(static_cast<size_t>(size));
        for (Py_ssize_t i = 0; i < size; ++i) {
            PyObject* item = PyList_GetItem(obj, i);
            result.push_back(convertFromPyObject<ElemType>(item));
        }
        return result;
    } else if constexpr (is_tuple<T>::value) {
        constexpr std::size_t N = std::tuple_size<T>::value;
        return tupleFromPyTupleImpl<T>(obj, std::make_index_sequence<N>{});
    } else {
        static_assert(sizeof(T) == 0, "convertFromPyObject: Unsupported type");
    }
}

// void型のための特殊化 (戻り値がない場合)
template<>
void convertFromPyObject<void>(PyObject* obj) {
    // PyObject* obj は通常 Py_None か、あるいは関数呼び出しの結果 (無視される)
    // 呼び出し側で Py_DECREF されるので、ここでは何もしない
    return;
}

} // namespace convert_helper