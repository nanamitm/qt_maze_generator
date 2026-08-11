#include "solvers/AStarSolver.h"

#include <cmath>

#include "MazeGrid.h"

double AStarSolver::heuristic(const QPoint& a, const QPoint& b)
{
    return std::abs(a.x() - b.x()) + std::abs(a.y() - b.y());
}

void AStarSolver::init(MazeGrid &grid, const QPoint &start, const QPoint &end)
{
    beginSearch(grid, start, end);

    m_open = {};
    m_closed.assign(grid.height(), std::vector<bool>(grid.width(), false));
    m_gScore.assign(grid.height(), std::vector<double>(grid.width(), 1e9));
    m_gScore[start.y()][start.x()] = 0.0;
    m_open.push(AStarNode{start, heuristic(start, end)});
}

bool AStarSolver::step(MazeGrid &grid)
{
    if (m_open.empty()) {
        return false;
    }

    const QPoint curr = m_open.top().pos;
    m_open.pop();

    // A cell can be queued more than once with different scores; the first one
    // out is the cheapest, the rest are stale.
    if (m_closed[curr.y()][curr.x()]) {
        return true;
    }
    m_closed[curr.y()][curr.x()] = true;

    if (visit(grid, curr)) {
        return false;
    }

    for (const QPoint &next : grid.walkableNeighbors(curr)) {
        if (m_closed[next.y()][next.x()]) {
            continue;
        }

        const double tentativeG = m_gScore[curr.y()][curr.x()] + 1.0;
        if (tentativeG >= m_gScore[next.y()][next.x()]) {
            continue;
        }

        m_parent[next.y()][next.x()] = curr;
        m_gScore[next.y()][next.x()] = tentativeG;
        grid[next.y()][next.x()].isFrontier = true;
        m_open.push(AStarNode{next, tentativeG + heuristic(next, m_end)});
    }

    return true;
}
