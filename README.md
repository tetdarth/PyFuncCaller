# PyFuncCaller

## 概要
C++からPythonの関数を簡単に呼び出せるようにラップしたヘッダファイルです。  
C++の型とPythonの標準的な型の相互変換を暗黙的に行えます。(※随時追加予定)

| C++ | Python |
| --- | --- |
| int | long |
| long | long |
| double | float(64bit) |
| char* | string |
| std::string | string |
| std::vector (n次元) | list (n次元) |

Pythonの複数戻り値はstd::tuple<>として受け取ることができます。

## 使い方

### 1. Python.hのセットアップ

#### Windowsの場合

1. **Python本体をインストール**  
   [公式サイト](https://www.python.org/downloads/)からPythonをインストールしてください。

2. **Pythonのインクルードパスとライブラリパスを確認**  
   例:  
   - インクルード: `C:\Users\ユーザー名\AppData\Local\Programs\Python\Python3x\include`
   - ライブラリ: `C:\Users\ユーザー名\AppData\Local\Programs\Python\Python3x\libs`

3. **プロジェクトのビルド設定に追加**  
   - インクルードディレクトリに`include`を追加
   - ライブラリディレクトリに`libs`を追加
   - リンカに`python3x.lib`（例: `python311.lib`）を追加

4. **環境変数PATHにPythonのDLLがあるディレクトリを追加**  
   例:  
   `C:\Users\ユーザー名\AppData\Local\Programs\Python\Python3x\`

#### Linux/macOSの場合

1. **Python開発パッケージをインストール**  
   - Ubuntu:  
     ```sh
     sudo apt-get install python3-dev
     ```
   - macOS (Homebrew):  
     ```sh
     brew install python
     ```

2. **g++やclang++でビルド時に`-I`と`-lpython3.x`を指定**  
   例:  
   ```sh
   g++ main.cpp -I/usr/include/python3.10 -lpython3.10
   ```

### 2. C++コードでのインクルード

```cpp
#include "$PyFuncCaller.h"
```

### 3. Pythonインタプリタの初期化

C++のmain関数の先頭で、Pythonインタプリタを初期化してください。

```cpp
#include "${header}/PyInitializer.h"

int main() {
    PFC::PyInitializer pyInit; // Pythonインタプリタの初期化
    // ...以降でPython関数を呼び出せます...
}
```

### 4. Python関数の呼び出し例

```cpp
PFC::PyFuncCaller add("mod", "add");
int result = add.call<int>(1, 2);
std::cout << "Result: " << result << std::endl;
```

### 5. 注意事項

- Python.hはC++のコードより前にインクルードしてください。
- Pythonスクリプト（例: mod.py）はC++実行ファイルと同じディレクトリ、または`sys.path`に含まれる場所に置いてください。
- Pythonインタプリタの初期化と終了は必ず行ってください（`PFC::PyInitializer`/`PFC::PyFinalizer`）。

---

これでC++からPython関数を安全に呼び出す準備が整います。