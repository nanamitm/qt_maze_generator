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
    ├── MazeModel.cpp          # グリッド状態と共通ヘルパー
    ├── MazeModel.h
    ├── MazeModelGeneration.cpp # DFS / Prim / 再帰分割
    └── MazeModelSolving.cpp    # BFS / DFS / A*
```

`resources/styles.qss` は Qt リソースとして実行ファイルに埋め込まれるため、配布時に別途コピーする必要はありません。

## Features

- DFS backtracker, Prim's algorithm, recursive division による迷路生成
- BFS, DFS, A* による経路探索
- アニメーション生成、即時生成、ステップ実行、一時停止、キャンセル
- マウス操作による壁の追加・削除、スタート/ゴール位置の移動
- ダークテーマのカスタム描画キャンバス
- Windows exe アイコン対応

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
