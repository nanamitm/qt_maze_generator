#include "solvers/DeadEndFillingSolver.h"

#include "MazeGrid.h"

int DeadEndFillingSolver::openDegree(const MazeGrid &grid, const QPoint &p) const
{
    int degree = 0;
    for (const QPoint &n : grid.walkableNeighbors(p)) {
        if (!m_filled[n.y()][n.x()]) {
            ++degree;
        }
    }
    return degree;
}

bool DeadEndFillingSolver::isFillable(const MazeGrid &grid, const QPoint &p) const
{
    if (grid.isWall(p) || m_filled[p.y()][p.x()]) {
        return false;
    }
    // Filling either endpoint would erase the very route being looked for.
    if (p == m_start || p == m_end) {
        return false;
    }
    return openDegree(grid, p) <= 1;
}

void DeadEndFillingSolver::init(MazeGrid &grid, const QPoint &start, const QPoint &end)
{
    beginSearch(grid, start, end);

    m_filled.assign(grid.height(), std::vector<bool>(grid.width(), false));
    m_pending.clear();
    m_finished = false;

    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            const QPoint p(x, y);
            if (isFillable(grid, p)) {
                m_pending.push_back(p);
            }
        }
    }
}

bool DeadEndFillingSolver::step(MazeGrid &grid)
{
    if (m_finished) {
        return false;
    }

    if (!m_pending.empty()) {
        const QPoint curr = m_pending.front();
        m_pending.pop_front();

        // The queue holds candidates, not certainties: a cell may have been
        // filled already, or may have gained a neighbour since being queued.
        if (!isFillable(grid, curr)) {
            return true;
        }

        m_filled[curr.y()][curr.x()] = true;
        grid[curr.y()][curr.x()].isVisited = true;

        // Plugging a dead end can turn its one neighbour into the next one.
        for (const QPoint &next : grid.walkableNeighbors(curr)) {
            if (isFillable(grid, next)) {
                m_pending.push_back(next);
            }
        }
        return true;
    }

    // Nothing left to plug, so whatever is still open is the route.
    int length = 0;
    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            if (grid.isWall(QPoint(x, y)) || m_filled[y][x]) {
                continue;
            }
            grid[y][x].isSolution = true;
            ++length;
        }
    }

    setPathLength(length);
    m_finished = true;
    return false;
}
