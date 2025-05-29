# PyFuncCaller
## 概要
C++からPythonの関数を簡単に呼び出せるようにラップしたヘッダファイル。  
C++の型とPythonの標準的な型の相互変換を暗黙的に行えます。  
(※随時対応予定)

| C++型                       | Python型           |
|-----------------------------|--------------------|
| bool                        | bool               |
| int, long                   | int                |
| double                      | float              |
| const char*, std::string    | str                |
| std::vector<T> (n次元)      | list (n次元)       |
| std::tuple<Ts...>           | tuple              |
| void                        | None               |

## 導入方法
簡単に言えばPython.hを使えるようにすれば動きます。

### Python.hのセットアップ

#### Windowsの場合

1. **Python本体をインストール**  
   [公式サイト](https://www.python.org/downloads/)からPythonをインストールします。

> [!IMPORTANT]
> Pythonのインストール時に「Add python.exe to PATH」にチェックを入れてインストールしてください。
> <img width="656" alt="image.png (87.8 kB)" src="https://img.esa.io/uploads/production/attachments/21659/2025/05/29/168660/c00d7da8-5600-421a-8982-9fca1aaff139.png">


2. **Pythonのインクルードパスとライブラリパスを確認**  
   例:  
   - インクルード: `C:\Users\ユーザー名\AppData\Local\Programs\Python\Python3x\include`
   - ライブラリ: `C:\Users\ユーザー名\AppData\Local\Programs\Python\Python3x\libs`

3. **プロジェクトのビルド設定に追加**  
   - インクルードディレクトリに`include`を追加
   - ライブラリディレクトリに`libs`を追加
   - リンカに`python3x.lib`（例: `python311.lib`）を追加

> [!TIP]
> VisualStudioプロジェクトの場合は、ソリューションのプロパティから
> - `構成プロパティ` > `C/C++` > `追加のインクルードディレクトリ`に`~/include`を追加
> - `構成プロパティ` > `V/C++ディレクトリ` > `ライブラリディレクトリ`に`~/libs`を追加
> - `構成プロパティ` > `リンカー` > `入力` > `追加の依存ファイル`に`python3x.lib`を追加

4. **環境変数PATHにPythonのDLLがあるディレクトリを追加**  
    <span style="color: red;">※上記の手順通りにインストールしていれば、この作業はする必要がありません。</span>
   例:  
   `C:\Users\ユーザー名\AppData\Local\Programs\Python\Python3x\`

> [!IMPORTANT]
> Minicondaやvenv等の仮想環境にpythonをインストールして実行することも可能です。
> ただし、`python3x.lib`がgccによるコンパイルに対応していない場合がありますので、msvcでコンパイルするのが無難です。
> また、`python3x_d.lib`が存在しない場合はDebugビルドが通らないため、注意してください。（入手する方法は各自調べてください）

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

## 使い方
以下Windowsでの使用を前提に話を進めます。（OSが違う人は適宜自分のOSに合うように調整）
### 準備
1. **PyFuncCaller.hのダウンロード**  
    [リポジトリ](https://github.com/tetdarth/PyFuncCaller)を`git clone`または.zipでダウンロードして任意の場所に配置。
2. **cppファイルでのインクルード**
    プロジェクトのインクルードディレクトリに、cloneまたは解凍した`~/PyFuncCaller/include`を追加。
    正常に`include`出来ていれば成功。
    ```cpp
    #include "PyFuncCaller.h"
    /*
    #include "../include/PyFuncCaller.h" // 相対パスでもinclude可能
    */
    ```
3. **pythonインタプリタの初期化**
  main()内で**一度だけ**pythonインタプリタを初期化する必要があります。（複数回呼び出すとエラーが発生します）
    PyFuncCaller.hに属するクラスはpfc名前空間に存在します。
    ```cpp
    #include "../include/PyFuncCaller.h"
    
    int main() {
        pfc::PyInitializer pyInit;   // Pythonインタプリタを初期化
        pyInit.addModuleDir("${相対 or 絶対パス}");   // Pythonモジュールのディレクトリを追加
        // ...以降でPython関数を呼び出せます...
    }
    ```

### cppからpythonモジュールの呼び出し
#### 基本的な使い方例
1. **引数が`int`二つ、返り値が`int`の単純な関数**
    python側で`add()`モジュールを作成した場合（example.py）
    ```python : example.py
    def add(x : int, y : int) -> int:
        return x + y
     ```
    `pfc::PyFuncCaller`のコンストラクタは、第一引数に`モジュール名`、第二引数に`関数名`を文字列で入れて紐づけします。
    pfc::PyFuncCaller.call<<span style="color: green; ">戻り値の型</span>>(<span style="color: blue; ">引数</span>)で関数を呼び出せます。
    ```cpp : example.cpp
    pfc::PyFuncCaller add("example", "add");
    int result = add.call<int>(1, 2);
    std::cout << "add(1, 2) = " << result << std::endl; // add(1, 2) = 3
    ```
2. **返り値が`void`の関数**
    ```python : example.py
    def isBool(b):
        print("PyTrue") if b else print("PyFalse")
    ```
    ```cpp : example.cpp
    pfc::PyFuncCaller isBool("example", "isBool");
    isBool.call<void>(false); // PyFalse
    ```
3. **返り値がstd::vectorの関数**
    `std::vector`はpythonでは`list`に変換されます。
返り値で`ndarray`は`torch.tensor`等の配列を`list`に型変換してください。
    ```python : example.py
    def getList(start, end, interval=1):
        return np.arange(start, end, interval).tolist()
    ```
    ```cpp : example.cpp
    pfc::PyFuncCaller getList("example", "getList");
    auto listResult1 = getList.call<std::vector<int>>(0, 10);
    for (const auto& item : listResult1) {
        std::cout << item << ", ";   // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    }
    
    auto listResult2 = getList.call<std::vector<double>>(0, 1, 0.2);
    for (const auto& item : listResult2) {
        std::cout << item << ", ";   // 0, 0.2, 0.4, 0.6, 0.8,
    }
    ```

4. **返り値が複数ある場合**
複数の返り値は`std::tuple<>`で受け取れます。
    ```python : example.py
    def getTuple(n: int, string: str) -> tuple[int, str]:
    return n, string
    ```
    `std::tuple<>`で受け取った要素は、`std::get<>()`でアンパックできます。
    ```cpp : example.cpp
    pfc::PyFuncCaller getTuple("example", "getTuple");
    auto tupleResult = getTuple.call<std::tuple<int, std::string>>(42, "Hello");
    std::cout << "getTuple(42, 'Hello') = (" 
              << std::get<0>(tupleResult) << ", " 
              << std::get<1>(tupleResult) << ")" << std::endl; // getTuple(42, 'Hello') = (42, Hello)
    ```

5. **std::vectorの扱い方**
`std::vector`はpythonでは`list`に変換されます。そのため、引数と戻り値を全て`list`にする必要があります。
    ```python : example.py
    def matMultiply(x : list, y : list) -> tuple[list, list, list, list]:
        x = np.array(x)
        y = np.array(y)
        return (x*x).tolist(), (x*y).tolist(), (y*x).tolist(), (y*y).tolist()
    ```
    `std::tuple<matrix, matrix, ...>`等で、複数のn次元配列を返り値にすることも可能です。
    ```cpp
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
    ```
