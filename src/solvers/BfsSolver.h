#ifndef BFSSOLVER_H
#define BFSSOLVER_H

#include <QPoint>
#include <deque>

#include "MazeSolver.h"

// Breadth-first search. Expands the frontier evenly in all directions, so the
// first time it reaches the goal it has taken the shortest route.
class BfsSolver : public MazeSolver {
public:
    void init(MazeGrid &grid, const QPoint &start, const QPoint &end) override;
    bool step(MazeGrid &grid) override;

private:
    std::deque<QPoint> m_queue;
};

#endif // BFSSOLVER_H
