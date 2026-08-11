# Qt Maze Generator

Qt6/C++ で作った迷路自動生成・探索アプリです。

![Qt Maze Generator screenshot](docs/screenshot.svg)

## Project Layout

```text
.
├── .github/
│   └── workflows/
│       └── windows-build.yml
├── CMakeLists.txt
├── LICENSE
├── README.md
├── docs/
│   └── screenshot.svg
├── resources/
│   ├── app.ico
│   ├── app-icon.png
│   ├── app.rc
│   └── styles.qss
└── src/
    ├── main.cpp
    ├── MainWindow.cpp
    ├── MainWindow.h
    ├── MazeCanvas.cpp
    ├── MazeCanvas.h
    ├── MazeGrid.h / .cpp       # グリッド実体と掘削ヘルパー
    ├── MazeGenerator.h         # 生成器インターフェース
    ├── MazeModel.cpp / .h      # 状態管理とシグナル
    ├── MazeModelGeneration.cpp # 生成の駆動
    ├── MazeModelSolving.cpp    # BFS / DFS / A*
    └── generators/             # アルゴリズム13種 + カタログ
```

`resources/styles.qss` は Qt リソースとして実行ファイルに埋め込まれるため、配布時に別途コピーする必要はありません。

## Features

- 13種の迷路生成アルゴリズム（下記）
- BFS, DFS, A* による経路探索
- アニメーション生成、即時生成、ステップ実行、一時停止、キャンセル
- シード指定による再現可能な生成（同じシードでアルゴリズムを比較可能）
- マウス操作による壁の追加・削除、スタート/ゴール位置の移動
- ダークテーマのカスタム描画キャンバス
- Windows exe アイコン対応

## Generators

同じシードで全種を見比べられます。袋小路率・分岐点率・最長経路は 88×47 = 4136 マスでの実測値です。

| アルゴリズム | 袋小路 | 分岐点 | 最長経路 |
|---|---|---|---|
| Recursive Backtracker (DFS) | 10.5% | 10.3% | 1494 |
| Hunt-and-Kill | 9.6% | 9.4% | 604 |
| Prim's Algorithm | 34.9% | 29.0% | 261 |
| Kruskal's Algorithm | 30.5% | 26.2% | 304 |
| Boruvka's Algorithm | 27.4% | 24.8% | 372 |
| Growing Tree | 15.3% | 14.6% | 457 |
| Growing Tree (oldest first) | 2.1% | 2.0% | 221 |
| Wilson (Loop-Erased Walk) | 29.3% | 25.3% | 362 |
| Aldous-Broder (Random Walk) | 28.8% | 25.4% | 527 |
| Eller's Algorithm | 29.8% | 25.8% | 327 |
| Sidewinder | 28.3% | 25.3% | 233 |
| Binary Tree | 25.2% | 25.1% | 237 |
| Recursive Division | 15.4% | 14.4% | 546 |

アルゴリズムの追加は `MazeGenerator` を継承したクラス1つと、`src/generators/GeneratorCatalog.cpp` への1行で完結します。UI と self-test はカタログを読むため改修不要です。

## Requirements

- Qt 6
- CMake 3.21 or later
- Ninja or another CMake generator
- C++17 compiler

## Build on Windows

```powershell
C:\Qt\Tools\CMake_64\bin\cmake.exe -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\mingw_64
C:\Qt\Tools\CMake_64\bin\cmake.exe --build build --config Release
```

`build/` は生成物なので Git 管理から除外しています。

## Run

```powershell
.\build\QtMazeGenerator.exe
```

## Self Test

```powershell
.\build\QtMazeGenerator.exe --self-test | Out-Host
if ($LASTEXITCODE -ne 0) { "self-test failed" }
```

The self-test runs all generator and solver combinations and exits with a non-zero code if any path cannot be solved. It also checks that the embedded stylesheet resolves. No display is required, so it can run on a headless machine.

`| Out-Host` は必須です。実行ファイルは GUI サブシステムでリンクされているため、PowerShell から直接呼ぶと終了を待たず `$LASTEXITCODE` も更新されません（パイプすると待機します）。

## CI

`.github/workflows/windows-build.yml` builds the application on GitHub Actions for Windows and runs the self-test.
