#ifndef WALLFOLLOWERSOLVER_H
#define WALLFOLLOWERSOLVER_H

#include <QPoint>
#include <vector>

#include "MazeSolver.h"
#include "solvers/WalkPath.h"

// Wall follower, right-hand rule. Keeps one hand on the wall and walks, which
// needs no memory of the maze at all: at each cell it just tries right, then
// straight, then left, then back.
//
// It is guaranteed to arrive in a generated maze, because a perfect maze has
// exactly one wall and following it eventually passes everything. Carve a loop
// by hand, though, and the enclosed wall becomes an island that the hand can
// circle forever; that case ends here by spotting a repeated position and
// heading rather than by running out of maze.
class WallFollowerSolver : public MazeSolver {
public:
    void init(MazeGrid &grid, const QPoint &start, const QPoint &end) override;
    bool step(MazeGrid &grid) override;

private:
    bool markStateSeen(const MazeGrid &grid);
    void finish(MazeGrid &grid);

    QPoint m_pos{1, 1};
    int m_heading = 0;
    WalkPath m_path;
    std::vector<bool> m_seenStates; // width * height * 4
    bool m_done = false;
};

#endif // WALLFOLLOWERSOLVER_H
