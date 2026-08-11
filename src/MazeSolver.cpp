#include "MazeSolver.h"

#include "MazeGrid.h"

void MazeSolver::beginSearch(MazeGrid &grid, const QPoint &start, const QPoint &end)
{
    m_start = start;
    m_end = end;
    m_pathLength = 0;
    m_parent.assign(grid.height(), std::vector<QPoint>(grid.width(), QPoint(-1, -1)));

    grid[start.y()][start.x()].isFrontier = true;
}

bool MazeSolver::visit(MazeGrid &grid, const QPoint &cell)
{
    grid[cell.y()][cell.x()].isFrontier = false;
    grid[cell.y()][cell.x()].isVisited = true;

    if (cell != m_end) {
        return false;
    }

    int length = 0;
    QPoint p = m_end;
    while (p != QPoint(-1, -1)) {
        grid[p.y()][p.x()].isSolution = true;
        p = m_parent[p.y()][p.x()];
        ++length;
    }
    m_pathLength = length;
    return true;
}
