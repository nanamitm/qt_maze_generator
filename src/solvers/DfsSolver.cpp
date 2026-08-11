#include "solvers/DfsSolver.h"

#include "MazeGrid.h"

void DfsSolver::init(MazeGrid &grid, const QPoint &start, const QPoint &end)
{
    beginSearch(grid, start, end);

    m_stack.clear();
    m_stack.push_back(start);
}

bool DfsSolver::step(MazeGrid &grid)
{
    if (m_stack.empty()) {
        return false;
    }

    const QPoint curr = m_stack.back();
    m_stack.pop_back();

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
        m_stack.push_back(next);
    }

    return true;
}
