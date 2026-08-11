#ifndef TREMAUXSOLVER_H
#define TREMAUXSOLVER_H

#include <QPoint>
#include <array>
#include <vector>

#include "MazeSolver.h"
#include "solvers/WalkPath.h"

// Tremaux's algorithm, the one you can run with chalk and no map. Every
// passage gets a mark each time it is walked, no passage marked twice is ever
// entered again, and unmarked passages come first. That is enough to guarantee
// finding the exit of any maze, loops and wall islands included, which is
// exactly where the wall follower gives up.
class TremauxSolver : public MazeSolver {
public:
    void init(MazeGrid &grid, const QPoint &start, const QPoint &end) override;
    bool step(MazeGrid &grid) override;

private:
    int chooseDirection(const MazeGrid &grid) const;
    void finish(MazeGrid &grid);

    QPoint m_pos{1, 1};
    int m_arrivalDir = -1; // direction taken to reach m_pos, -1 at the start
    bool m_done = false;

    // Marks per cell and direction, kept in step with the twin entry on the
    // far side, since a passage is one thing seen from either end.
    std::vector<std::vector<std::array<int, 4>>> m_marks;
    std::vector<std::vector<bool>> m_seenCell;
    WalkPath m_path;
};

#endif // TREMAUXSOLVER_H
