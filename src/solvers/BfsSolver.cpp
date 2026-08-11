#include "solvers/BfsSolver.h"

#include "MazeGrid.h"

void BfsSolver::init(MazeGrid &grid, const QPoint &start, const QPoint &end)
{
    beginSearch(grid, start, end);

    m_queue.clear();
    m_queue.push_back(start);
}

bool BfsSolver::step(MazeGrid &grid)
{
    if (m_queue.empty()) {
        return false;
    }

    const QPoint curr = m_queue.front();
    m_queue.pop_front();

    if (visit(grid, curr)) {
        return false;
    }

    for (const QPoint &next : grid.walkableNeighbors(curr)) {
        Cell &cell = grid[next.y()][next.x()];
        if (cell.isVisited || cell.isFrontier) {
            continue;
        }
        m_parent[next.y()][next.x()] = curr;
        cell.isFrontier = true;
        m_queue.push_back(next);
    }

    return true;
}
