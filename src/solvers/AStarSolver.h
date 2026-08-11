#ifndef ASTARSOLVER_H
#define ASTARSOLVER_H

#include <QPoint>
#include <functional>
#include <queue>
#include <vector>

#include "MazeSolver.h"

// Ordering for the open set: lowest estimated total cost first.
struct AStarNode {
    QPoint pos;
    double fScore;
    bool operator>(const AStarNode& other) const {
        return fScore > other.fScore;
    }
};

// A* search. Expands whichever cell looks cheapest once the distance already
// walked is added to a guess at the distance left, so it heads for the goal
// without giving up the shortest route.
class AStarSolver : public MazeSolver {
public:
    void init(MazeGrid &grid, const QPoint &start, const QPoint &end) override;
    bool step(MazeGrid &grid) override;

private:
    // Manhattan distance: never overestimates on a four-way grid, which is
    // what keeps the result optimal.
    static double heuristic(const QPoint& a, const QPoint& b);

    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> m_open;
    std::vector<std::vector<double>> m_gScore;
    std::vector<std::vector<bool>> m_closed;
};

#endif // ASTARSOLVER_H
