#ifndef MAZEMODEL_H
#define MAZEMODEL_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <vector>
#include <memory>
#include <random>

#include "MazeGenerator.h"
#include "MazeGrid.h"
#include "MazeSolver.h"

class MazeModel : public QObject {
    Q_OBJECT

public:
    explicit MazeModel(QObject *parent = nullptr);
    ~MazeModel() override;

    void setSize(int w, int h);
    int width() const { return m_grid.width(); }
    int height() const { return m_grid.height(); }

    const Cell& cell(int row, int col) const;
    void setWall(int row, int col, bool isWall);

    QPoint startPos() const { return m_startPos; }
    void setStartPos(const QPoint& pos);

    QPoint endPos() const { return m_endPos; }
    void setEndPos(const QPoint& pos);

    bool isGenerating() const { return m_generating; }
    bool isSolving() const { return m_solving; }
    bool isFinished() const { return m_finished; }
    bool hasGeneratedMaze() const { return m_hasGeneratedMaze; }

    bool isValid(const QPoint& p) const;
    bool isValid(int r, int c) const;

    // Generation is reseeded from this at every run, so the same seed and the
    // same algorithm always produce the same maze, and one seed can be used to
    // compare algorithms against each other.
    void setSeed(quint32 seed) { m_seed = seed; }
    quint32 seed() const { return m_seed; }

    // How many steps of the active generator or solver are worth showing in
    // one frame.
    int generatorStepScale() const;
    int solverStepScale() const;

    // Control functions. The generator is named by its catalog id.
    void initGeneration(const QString& generatorId);
    bool stepGeneration(); // Returns true if generation is still running, false if finished
    void generateInstant(const QString& generatorId);
    void cancelGeneration();

    // The solver is likewise named by its catalog id.
    void initSolving(const QString& solverId);
    bool stepSolving(); // Returns true if solving is still running, false if finished
    void solveInstant(const QString& solverId);
    void cancelSolving();

    void clearAll();        // Resets the grid to all walls
    void clearPathOnly();   // Clears solver/path highlights but keeps walls
    void resetToEmpty();    // Resets to all paths with outer borders as walls

signals:
    void gridChanged();
    void statusUpdated(const QString& status);

private:
    MazeGrid m_grid;

    QPoint m_startPos;
    QPoint m_endPos;

    bool m_generating = false;
    bool m_solving = false;
    bool m_finished = false;
    bool m_hasGeneratedMaze = false;

    std::unique_ptr<MazeGenerator> m_generator;
    QString m_generatorName;
    std::unique_ptr<MazeSolver> m_solver;
    QString m_solverName;

    // Random number generator
    std::mt19937 m_rng;
    quint32 m_seed = 0;

    // Helper functions
    QPoint nearestMazeAnchor(const QPoint& p) const;
    void carveEndpointConnection(const QPoint& p);
    void ensureEndpointConnections();
    void ensureEndReachable();
};

#endif // MAZEMODEL_H
