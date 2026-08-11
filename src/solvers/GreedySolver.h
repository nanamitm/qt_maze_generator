#ifndef GREEDYSOLVER_H
#define GREEDYSOLVER_H

#include <QPoint>
#include <functional>
#include <queue>
#include <vector>

#include "MazeSolver.h"
#include "solvers/BestFirst.h"

// Greedy best-first search. Scores a cell by the guess at the distance left and
// nothing else, so it makes straight for the goal and explores far less than A*
// does. It keeps no record of how far it has walked, which is why the route it
// finds is usually not the shortest one.
class GreedySolver : public MazeSolver {
public:
    void init(MazeGrid &grid, const QPoint &start, const QPoint &end) override;
    bool step(MazeGrid &grid) override;

private:
    std::priority_queue<BestFirstNode, std::vector<BestFirstNode>, std::greater<BestFirstNode>> m_open;
    std::vector<std::vector<bool>> m_closed;
};

#endif // GREEDYSOLVER_H
