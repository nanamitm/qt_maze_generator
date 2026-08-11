#ifndef DFSSOLVER_H
#define DFSSOLVER_H

#include <QPoint>
#include <vector>

#include "MazeSolver.h"

// Depth-first search. Follows one corridor as far as it goes before trying
// another, so it often reaches the goal early but by a route that wanders.
class DfsSolver : public MazeSolver {
public:
    void init(MazeGrid &grid, const QPoint &start, const QPoint &end) override;
    bool step(MazeGrid &grid) override;

private:
    std::vector<QPoint> m_stack;
};

#endif // DFSSOLVER_H
