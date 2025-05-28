# PyFuncCaller

## 概要
C++からpythonの関数を簡単に呼び出せるようにラップしたヘッダファイルです。  
C++の型とPythonの標準的な型の相互変換を暗示的に行えます。(※随時追加予定)

| C++ | Python |
| --- | --- |
| int | long |
| long | long |
| double | float(64bit) |
| char* | string |
| std::string | string |
| std::vector (n次元) | list (n次元) |

## 使い方
### Python.hのセットアップ
