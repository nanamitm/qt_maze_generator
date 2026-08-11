#include "solvers/WallFollowerSolver.h"

#include "MazeGrid.h"

namespace {

// North, east, south, west. Turning right is +1, left is +3, back is +2.
const QPoint kDirections[4] = {QPoint(0, -1), QPoint(1, 0), QPoint(0, 1), QPoint(-1, 0)};

} // namespace

void WallFollowerSolver::init(MazeGrid &grid, const QPoint &start, const QPoint &end)
{
    beginSearch(grid, start, end);

    m_pos = start;
    m_heading = 0;
    m_done = false;
    m_path.reset(grid, start);
    m_seenStates.assign(static_cast<size_t>(grid.width()) * grid.height() * 4, false);

    grid[start.y()][start.x()].isVisited = true;
}

bool WallFollowerSolver::markStateSeen(const MazeGrid &grid)
{
    const size_t index =
        (static_cast<size_t>(m_pos.y()) * grid.width() + m_pos.x()) * 4 + m_heading;
    if (m_seenStates[index]) {
        return false;
    }
    m_seenStates[index] = true;
    return true;
}

void WallFollowerSolver::finish(MazeGrid &grid)
{
    for (const QPoint &cell : m_path.cells()) {
        grid[cell.y()][cell.x()].isSolution = true;
    }
    setPathLength(static_cast<int>(m_path.cells().size()));
    m_done = true;
}

bool WallFollowerSolver::step(MazeGrid &grid)
{
    if (m_done) {
        return false;
    }

    if (m_pos == m_end) {
        finish(grid);
        return false;
    }

    // Repeating a position with the same heading means the hand is going round
    // a wall island and will never come off it.
    if (!markStateSeen(grid)) {
        return false;
    }

    // Right first, then straight, then left, then back the way we came.
    for (int turn : {1, 0, 3, 2}) {
        const int heading = (m_heading + turn) % 4;
        const QPoint next = m_pos + kDirections[heading];

        if (!grid.isValid(next) || grid.isWall(next)) {
            continue;
        }

        m_heading = heading;
        m_pos = next;
        m_path.moveTo(next);

        grid[next.y()][next.x()].isVisited = true;
        grid.clearFrontier();
        grid[next.y()][next.x()].isFrontier = true;

        if (m_pos == m_end) {
            finish(grid);
            return false;
        }
        return true;
    }

    return false; // walled in on every side
}
