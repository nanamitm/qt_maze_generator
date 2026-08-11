#ifndef MAZEMODEL_H
#define MAZEMODEL_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <vector>
#include <queue>
#include <deque>
#include <map>
#include <memory>
#include <random>

#include "MazeGenerator.h"
#include "MazeGrid.h"

enum class GeneratorType {
    DFS,
    Prim,
    Division
};

enum class SolverType {
    BFS,
    DFS,
    AStar
};

// Compare struct for A* priority queue
struct AStarNode {
    QPoint pos;
    double fScore;
    bool operator>(const AStarNode& other) const {
        return fScore > other.fScore;
    }
};

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

    // Control functions
    void initGeneration(GeneratorType type);
    bool stepGeneration(); // Returns true if generation is still running, false if finished
    void generateInstant(GeneratorType type);
    void cancelGeneration();

    void initSolving(SolverType type);
    bool stepSolving(); // Returns true if solving is still running, false if finished
    void solveInstant(SolverType type);
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
    SolverType m_activeSolveType = SolverType::BFS;

    // Random number generator
    std::mt19937 m_rng;

    // Helper functions
    double heuristic(const QPoint& p1, const QPoint& p2) const;
    QPoint nearestMazeAnchor(const QPoint& p) const;
    void carveEndpointConnection(const QPoint& p);
    void ensureEndpointConnections();
    void ensureEndReachable();

    // Solver states
    std::deque<QPoint> m_solveQueue;                  // BFS
    std::vector<QPoint> m_solveStack;                 // DFS
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> m_solveMinHeap; // A*
    std::vector<std::vector<QPoint>> m_parentMap;     // Parent pointer grid
    std::vector<std::vector<double>> m_gScore;        // A* g scores
    std::vector<std::vector<bool>> m_closedSet;       // Solver closed set
};

#endif // MAZEMODEL_H
