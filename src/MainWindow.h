#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include "MazeModel.h"
#include "MazeCanvas.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onGenerateClicked();
    void onInstantGenerateClicked();
    void onSolveClicked();
    void onInstantSolveClicked();
    void onPlayPauseClicked();
    void onStepClicked();
    void onClearPathClicked();
    void onClearAllClicked();
    void onResetEmptyClicked();
    void onTimerTick();
    void onSpeedChanged(int value);
    void onSizeChanged();
    void updateStatus(const QString& status);
    void updateUIStates();

private:
    void setupUI();
    void loadStyleSheet();
    int solvingStepsPerTick() const;

    MazeModel *m_model = nullptr;
    MazeCanvas *m_canvas = nullptr;
    QTimer *m_timer = nullptr;

    // UI Widgets
    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
    QComboBox *m_genAlgoCombo = nullptr;
    QComboBox *m_solveAlgoCombo = nullptr;
    QSlider *m_speedSlider = nullptr;
    QLabel *m_speedValLabel = nullptr;

    QPushButton *m_generateBtn = nullptr;
    QPushButton *m_instantGenBtn = nullptr;
    QPushButton *m_solveBtn = nullptr;
    QPushButton *m_instantSolveBtn = nullptr;
    QPushButton *m_playPauseBtn = nullptr;
    QPushButton *m_stepBtn = nullptr;
    QPushButton *m_clearPathBtn = nullptr;
    QPushButton *m_clearAllBtn = nullptr;
    QPushButton *m_resetEmptyBtn = nullptr;

    QLabel *m_statusLabel = nullptr;
};

#endif // MAINWINDOW_H
