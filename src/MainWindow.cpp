#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QSplitter>
#include <QGridLayout>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QFrame>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    loadStyleSheet();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTick);

    // Initial speed setting
    onSpeedChanged(m_speedSlider->value());

    updateUIStates();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Qt6 C++ Maze Generator & Solver");
    resize(1024, 768);

    m_model = new MazeModel(this);
    m_canvas = new MazeCanvas(this);
    m_canvas->setModel(m_model);

    connect(m_model, &MazeModel::statusUpdated, this, &MainWindow::updateStatus);

    // Main central splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    // --- Left Sidebar Panel (Scrollable) ---
    QScrollArea *scrollArea = new QScrollArea(splitter);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QFrame *sidebar = new QFrame(scrollArea);
    sidebar->setObjectName("sidebar");
    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(12, 12, 12, 12);
    sideLayout->setSpacing(8);
    scrollArea->setWidget(sidebar);

    // 1. Grid Size Settings
    QGroupBox *sizeGroup = new QGroupBox("Grid Settings", sidebar);
    QFormLayout *sizeForm = new QFormLayout(sizeGroup);
    sizeForm->setSpacing(6);

    m_widthSpin = new QSpinBox(sizeGroup);
    m_widthSpin->setRange(5, 199);
    m_widthSpin->setValue(31);
    m_widthSpin->setSingleStep(2); // Keep odd values
    connect(m_widthSpin, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onSizeChanged);

    m_heightSpin = new QSpinBox(sizeGroup);
    m_heightSpin->setRange(5, 199);
    m_heightSpin->setValue(31);
    m_heightSpin->setSingleStep(2); // Keep odd values
    connect(m_heightSpin, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onSizeChanged);

    sizeForm->addRow("Grid Width:", m_widthSpin);
    sizeForm->addRow("Grid Height:", m_heightSpin);
    sideLayout->addWidget(sizeGroup);

    // 2. Generator Settings
    QGroupBox *genGroup = new QGroupBox("Maze Generator", sidebar);
    QVBoxLayout *genLayout = new QVBoxLayout(genGroup);
    genLayout->setSpacing(6);

    m_genAlgoCombo = new QComboBox(genGroup);
    m_genAlgoCombo->addItem("DFS (Backtracker)", static_cast<int>(GeneratorType::DFS));
    m_genAlgoCombo->addItem("Prim's Algorithm", static_cast<int>(GeneratorType::Prim));
    m_genAlgoCombo->addItem("Recursive Division", static_cast<int>(GeneratorType::Division));
    genLayout->addWidget(m_genAlgoCombo);

    QHBoxLayout *genButtonsLayout = new QHBoxLayout();
    genButtonsLayout->setSpacing(6);

    m_generateBtn = new QPushButton("Generate", genGroup);
    m_generateBtn->setObjectName("generateBtn");
    connect(m_generateBtn, &QPushButton::clicked, this, &MainWindow::onGenerateClicked);
    genButtonsLayout->addWidget(m_generateBtn);

    m_instantGenBtn = new QPushButton("Instant", genGroup);
    connect(m_instantGenBtn, &QPushButton::clicked, this, &MainWindow::onInstantGenerateClicked);
    genButtonsLayout->addWidget(m_instantGenBtn);
    genLayout->addLayout(genButtonsLayout);
    sideLayout->addWidget(genGroup);

    // 3. Solver Settings
    QGroupBox *solveGroup = new QGroupBox("Maze Solver", sidebar);
    QVBoxLayout *solveLayout = new QVBoxLayout(solveGroup);
    solveLayout->setSpacing(6);

    m_solveAlgoCombo = new QComboBox(solveGroup);
    m_solveAlgoCombo->addItem("Breadth-First Search (BFS)", static_cast<int>(SolverType::BFS));
    m_solveAlgoCombo->addItem("Depth-First Search (DFS)", static_cast<int>(SolverType::DFS));
    m_solveAlgoCombo->addItem("A* Search Algorithm", static_cast<int>(SolverType::AStar));
    solveLayout->addWidget(m_solveAlgoCombo);

    QHBoxLayout *solveButtonsLayout = new QHBoxLayout();
    solveButtonsLayout->setSpacing(6);

    m_solveBtn = new QPushButton("Solve", solveGroup);
    m_solveBtn->setObjectName("solveBtn");
    connect(m_solveBtn, &QPushButton::clicked, this, &MainWindow::onSolveClicked);
    solveButtonsLayout->addWidget(m_solveBtn);

    m_instantSolveBtn = new QPushButton("Instant", solveGroup);
    connect(m_instantSolveBtn, &QPushButton::clicked, this, &MainWindow::onInstantSolveClicked);
    solveButtonsLayout->addWidget(m_instantSolveBtn);
    solveLayout->addLayout(solveButtonsLayout);
    sideLayout->addWidget(solveGroup);

    // 4. Control Settings
    QGroupBox *controlGroup = new QGroupBox("Simulation Controls", sidebar);
    QVBoxLayout *ctrlLayout = new QVBoxLayout(controlGroup);
    ctrlLayout->setSpacing(6);

    // Speed Slider
    QHBoxLayout *speedLayout = new QHBoxLayout();
    QLabel *speedLabel = new QLabel("Delay:", controlGroup);
    m_speedSlider = new QSlider(Qt::Horizontal, controlGroup);
    m_speedSlider->setRange(1, 250);
    m_speedSlider->setValue(15);
    m_speedValLabel = new QLabel("15 ms", controlGroup);
    m_speedValLabel->setMinimumWidth(70);
    speedLayout->addWidget(speedLabel);
    speedLayout->addWidget(m_speedSlider);
    speedLayout->addWidget(m_speedValLabel);
    ctrlLayout->addLayout(speedLayout);
    connect(m_speedSlider, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);

    // Play/Pause and Step controls
    QHBoxLayout *playStepLayout = new QHBoxLayout();
    m_playPauseBtn = new QPushButton("Pause", controlGroup);
    m_playPauseBtn->setObjectName("playPauseBtn");
    connect(m_playPauseBtn, &QPushButton::clicked, this, &MainWindow::onPlayPauseClicked);

    m_stepBtn = new QPushButton("Step", controlGroup);
    connect(m_stepBtn, &QPushButton::clicked, this, &MainWindow::onStepClicked);

    playStepLayout->addWidget(m_playPauseBtn);
    playStepLayout->addWidget(m_stepBtn);
    ctrlLayout->addLayout(playStepLayout);

    QGridLayout *clearLayout = new QGridLayout();
    clearLayout->setSpacing(6);

    m_clearPathBtn = new QPushButton("Clear Path", controlGroup);
    connect(m_clearPathBtn, &QPushButton::clicked, this, &MainWindow::onClearPathClicked);
    clearLayout->addWidget(m_clearPathBtn, 0, 0);

    m_clearAllBtn = new QPushButton("Fill Walls", controlGroup);
    m_clearAllBtn->setObjectName("clearBtn");
    connect(m_clearAllBtn, &QPushButton::clicked, this, &MainWindow::onClearAllClicked);
    clearLayout->addWidget(m_clearAllBtn, 0, 1);

    m_resetEmptyBtn = new QPushButton("Empty Grid", controlGroup);
    connect(m_resetEmptyBtn, &QPushButton::clicked, this, &MainWindow::onResetEmptyClicked);
    clearLayout->addWidget(m_resetEmptyBtn, 1, 0, 1, 2);
    ctrlLayout->addLayout(clearLayout);

    sideLayout->addWidget(controlGroup);

    // 5. Status Panel
    m_statusLabel = new QLabel("Status: Idle", sidebar);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setWordWrap(true);
    sideLayout->addWidget(m_statusLabel);

    sideLayout->addStretch();

    // Set minimum sizes to prevent layout collapse
    scrollArea->setMinimumWidth(280);
    m_canvas->setMinimumSize(400, 400);

    // Add widgets to splitter
    splitter->addWidget(scrollArea);
    splitter->addWidget(m_canvas);

    // Set initial splitter sizes
    splitter->setSizes({300, 724});

    // Set splitter ratios
    splitter->setStretchFactor(0, 0); // Sidebar takes min width
    splitter->setStretchFactor(1, 1); // Canvas expands
}

void MainWindow::loadStyleSheet()
{
    // Search stylesheet in local path or target build folder path
    QStringList paths = {
        QCoreApplication::applicationDirPath() + "/styles.qss",
        QCoreApplication::applicationDirPath() + "/resources/styles.qss",
        QDir::currentPath() + "/styles.qss",
        QDir::currentPath() + "/resources/styles.qss",
        QDir::currentPath() + "/qt_maze_generator/styles.qss",
        QDir::currentPath() + "/qt_maze_generator/resources/styles.qss",
        "../styles.qss"
    };

    for (const auto& path : paths) {
        QFile file(path);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QString styleSheet = QLatin1String(file.readAll());
            setStyleSheet(styleSheet);
            break;
        }
    }
}

void MainWindow::onGenerateClicked()
{
    if (m_model->isGenerating()) {
        m_timer->stop();
        m_model->cancelGeneration();
        updateUIStates();
        return;
    }

    m_timer->stop();
    GeneratorType genType = static_cast<GeneratorType>(m_genAlgoCombo->currentData().toInt());
    m_model->initGeneration(genType);
    m_timer->start();
    updateUIStates();
}

void MainWindow::onInstantGenerateClicked()
{
    m_timer->stop();
    GeneratorType genType = static_cast<GeneratorType>(m_genAlgoCombo->currentData().toInt());
    m_model->generateInstant(genType);
    updateUIStates();
}

void MainWindow::onSolveClicked()
{
    if (m_model->isSolving()) {
        m_timer->stop();
        m_model->cancelSolving();
        updateUIStates();
        return;
    }

    m_timer->stop();
    SolverType solveType = static_cast<SolverType>(m_solveAlgoCombo->currentData().toInt());
    m_model->initSolving(solveType);
    m_timer->start();
    updateUIStates();
}

void MainWindow::onInstantSolveClicked()
{
    m_timer->stop();
    SolverType solveType = static_cast<SolverType>(m_solveAlgoCombo->currentData().toInt());
    m_model->solveInstant(solveType);
    updateUIStates();
}

void MainWindow::onPlayPauseClicked()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        m_statusLabel->setText("Status: Paused");
    } else {
        if (m_model->isGenerating() || m_model->isSolving()) {
            m_timer->start();
            m_statusLabel->setText("Status: Running...");
        }
    }
    updateUIStates();
}

void MainWindow::onStepClicked()
{
    m_timer->stop();
    if (m_model->isGenerating()) {
        m_model->stepGeneration();
    } else if (m_model->isSolving()) {
        m_model->stepSolving();
    }
    updateUIStates();
}

void MainWindow::onClearPathClicked()
{
    m_timer->stop();
    m_model->clearPathOnly();
    m_statusLabel->setText("Status: Paths Cleared");
    updateUIStates();
}

void MainWindow::onClearAllClicked()
{
    m_timer->stop();
    m_model->clearAll();
    m_statusLabel->setText("Status: Grid Cleared");
    updateUIStates();
}

void MainWindow::onResetEmptyClicked()
{
    m_timer->stop();
    m_model->resetToEmpty();
    m_statusLabel->setText("Status: Reset to Empty Grid");
    updateUIStates();
}

void MainWindow::onTimerTick()
{
    bool continues = false;
    if (m_model->isGenerating()) {
        continues = m_model->stepGeneration();
    } else if (m_model->isSolving()) {
        const int steps = solvingStepsPerTick();
        for (int i = 0; i < steps; ++i) {
            continues = m_model->stepSolving();
            if (!continues) {
                break;
            }
        }
    }

    if (!continues) {
        m_timer->stop();
        updateUIStates();
    }
}

void MainWindow::onSpeedChanged(int value)
{
    const int solveSteps = solvingStepsPerTick();
    if (solveSteps > 1) {
        m_speedValLabel->setText(QString("%1 ms x%2").arg(value).arg(solveSteps));
    } else {
        m_speedValLabel->setText(QString("%1 ms").arg(value));
    }
    m_timer->setInterval(value);
}

int MainWindow::solvingStepsPerTick() const
{
    if (!m_speedSlider) {
        return 1;
    }

    const int delay = m_speedSlider->value();
    if (delay <= 5) return 32;
    if (delay <= 10) return 24;
    if (delay <= 20) return 16;
    if (delay <= 40) return 8;
    if (delay <= 80) return 4;
    if (delay <= 140) return 2;
    return 1;
}

void MainWindow::onSizeChanged()
{
    // Prevent size changes while running
    if (m_model->isGenerating() || m_model->isSolving()) {
        return;
    }

    int w = m_widthSpin->value();
    int h = m_heightSpin->value();

    // Force odd values
    if (w % 2 == 0) {
        w++;
        m_widthSpin->setValue(w);
    }
    if (h % 2 == 0) {
        h++;
        m_heightSpin->setValue(h);
    }

    m_model->setSize(w, h);
    m_statusLabel->setText(QString("Grid resized to %1 x %2").arg(w).arg(h));
}

void MainWindow::updateStatus(const QString& status)
{
    m_statusLabel->setText(QString("Status: %1").arg(status));
}

void MainWindow::updateUIStates()
{
    bool isRunning = m_timer->isActive();
    bool isSimulating = m_model->isGenerating() || m_model->isSolving();
    bool canSolve = m_model->hasGeneratedMaze() && !isSimulating;

    // Disable configurations while simulating
    m_widthSpin->setEnabled(!isSimulating);
    m_heightSpin->setEnabled(!isSimulating);
    m_genAlgoCombo->setEnabled(!isSimulating);
    m_solveAlgoCombo->setEnabled(canSolve);

    // Generate becomes Cancel while an animated generation is in progress.
    m_generateBtn->setEnabled(!m_model->isSolving());
    m_generateBtn->setText(m_model->isGenerating() ? "Cancel" : "Generate");
    m_instantGenBtn->setEnabled(!isSimulating);
    // Solve becomes Cancel while an animated solve is in progress.
    m_solveBtn->setEnabled(m_model->isSolving() || canSolve);
    m_solveBtn->setText(m_model->isSolving() ? "Cancel" : "Solve");
    m_instantSolveBtn->setEnabled(canSolve);

    // Clear buttons disabled when running
    m_clearPathBtn->setEnabled(!isRunning);
    m_clearAllBtn->setEnabled(!isRunning);
    m_resetEmptyBtn->setEnabled(!isRunning);

    // Play/Pause button state
    if (isSimulating) {
        m_playPauseBtn->setEnabled(true);
        m_stepBtn->setEnabled(!isRunning);
        if (isRunning) {
            m_playPauseBtn->setText("Pause");
        } else {
            m_playPauseBtn->setText("Resume");
        }
    } else {
        m_playPauseBtn->setEnabled(false);
        m_playPauseBtn->setText("Pause");
        m_stepBtn->setEnabled(false);
    }
}
