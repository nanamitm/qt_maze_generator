#include "solvers/GreedySolver.h"

#include "MazeGrid.h"

void GreedySolver::init(MazeGrid &grid, const QPoint &start, const QPoint &end)
{
    beginSearch(grid, start, end);

    m_open = {};
    m_closed.assign(grid.height(), std::vector<bool>(grid.width(), false));
    m_open.push(BestFirstNode{start, manhattan(start, end)});
}

bool GreedySolver::step(MazeGrid &grid)
{
    if (m_open.empty()) {
        return false;
    }

    const QPoint curr = m_open.top().pos;
    m_open.pop();

    if (m_closed[curr.y()][curr.x()]) {
        return true;
    }
    m_closed[curr.y()][curr.x()] = true;

    if (visit(grid, curr)) {
        return false;
    }

    for (const QPoint &next : grid.walkableNeighbors(curr)) {
        Cell &cell = grid[next.y()][next.x()];
        if (m_closed[next.y()][next.x()] || cell.isFrontier) {
            continue;
        }

        // Whichever cell reaches it first becomes its parent. With no cost
        // being tracked there is nothing to compare a later route against, so
        // there is no relaxation step here, unlike in A*.
        m_parent[next.y()][next.x()] = curr;
        cell.isFrontier = true;
        m_open.push(BestFirstNode{next, manhattan(next, m_end)});
    }

    return true;
}
