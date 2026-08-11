#ifndef MAZESOLVER_H
#define MAZESOLVER_H

#include <QPoint>
#include <vector>

class MazeGrid;

// The solver counterpart of MazeGenerator: init() prepares the search, step()
// advances it by one visible unit of work and returns false once the search is
// over, whether or not it found a path. Animated and instant solving both drive
// this, so each algorithm is written once.
class MazeSolver {
public:
    MazeSolver() = default;
    virtual ~MazeSolver() = default;

    MazeSolver(const MazeSolver&) = delete;
    MazeSolver& operator=(const MazeSolver&) = delete;

    virtual void init(MazeGrid &grid, const QPoint &start, const QPoint &end) = 0;
    virtual bool step(MazeGrid &grid) = 0;

    // Multiplier for the animation step budget, matching MazeGenerator.
    virtual int stepScale() const { return 1; }

    // Zero until a path has been found.
    int pathLength() const { return m_pathLength; }

protected:
    // Clears the parent map and puts the start cell on the frontier.
    void beginSearch(MazeGrid &grid, const QPoint &start, const QPoint &end);

    // Takes a cell off the frontier and marks it visited. When it is the goal,
    // paints the path back to the start and returns true, which is every
    // algorithm's signal that the search is done.
    bool visit(MazeGrid &grid, const QPoint &cell);

    // Paints the chain of cells reachable from `from` by following `parents`,
    // and returns how many it painted. Solvers that build their path some
    // other way use these two directly instead of visit().
    int paintChain(MazeGrid &grid, const std::vector<std::vector<QPoint>> &parents, const QPoint &from);
    void setPathLength(int length) { m_pathLength = length; }

    QPoint m_start{-1, -1};
    QPoint m_end{-1, -1};
    std::vector<std::vector<QPoint>> m_parent;

private:
    int m_pathLength = 0;
};

#endif // MAZESOLVER_H
