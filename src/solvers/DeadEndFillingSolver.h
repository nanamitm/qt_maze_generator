#ifndef DEADENDFILLINGSOLVER_H
#define DEADENDFILLINGSOLVER_H

#include <QPoint>
#include <deque>
#include <vector>

#include "MazeSolver.h"

// Dead-end filling. Rather than searching for the route, it plugs every dead
// end and keeps plugging the ones that appear behind them; in a perfect maze
// what is left when nothing is left to plug is exactly the route. It never
// looks at the start or the goal while doing so, which is what makes it feel
// unlike every other solver here.
//
// It marks cells as filled rather than walling them off, so the maze itself is
// left untouched and can be solved again by something else.
class DeadEndFillingSolver : public MazeSolver {
public:
    void init(MazeGrid &grid, const QPoint &start, const QPoint &end) override;
    bool step(MazeGrid &grid) override;

    int stepScale() const override { return 3; }

private:
    // Open neighbours that have not been filled yet.
    int openDegree(const MazeGrid &grid, const QPoint &p) const;
    bool isFillable(const MazeGrid &grid, const QPoint &p) const;

    std::vector<std::vector<bool>> m_filled;
    std::deque<QPoint> m_pending;
    bool m_finished = false;
};

#endif // DEADENDFILLINGSOLVER_H
