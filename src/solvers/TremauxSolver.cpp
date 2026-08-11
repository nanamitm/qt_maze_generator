#include "solvers/TremauxSolver.h"

#include "MazeGrid.h"

namespace {

const QPoint kDirections[4] = {QPoint(0, -1), QPoint(1, 0), QPoint(0, 1), QPoint(-1, 0)};

int opposite(int direction) { return (direction + 2) % 4; }

} // namespace

void TremauxSolver::init(MazeGrid &grid, const QPoint &start, const QPoint &end)
{
    beginSearch(grid, start, end);

    m_pos = start;
    m_arrivalDir = -1;
    m_done = false;

    m_marks.assign(grid.height(), std::vector<std::array<int, 4>>(grid.width(), {0, 0, 0, 0}));
    m_seenCell.assign(grid.height(), std::vector<bool>(grid.width(), false));
    m_path.reset(grid, start);

    grid[start.y()][start.x()].isVisited = true;
}

int TremauxSolver::chooseDirection(const MazeGrid &grid) const
{
    std::vector<int> unmarked;
    std::vector<int> markedOnce;

    for (int d = 0; d < 4; ++d) {
        const QPoint next = m_pos + kDirections[d];
        if (!grid.isValid(next) || grid.isWall(next)) {
            continue;
        }
        const int marks = m_marks[m_pos.y()][m_pos.x()][d];
        if (marks == 0) {
            unmarked.push_back(d);
        } else if (marks == 1) {
            markedOnce.push_back(d);
        }
    }

    const int back = (m_arrivalDir >= 0) ? opposite(m_arrivalDir) : -1;

    if (!m_seenCell[m_pos.y()][m_pos.x()]) {
        // First time here: any unmarked passage will do, and a dead end sends
        // us straight back.
        if (!unmarked.empty()) return unmarked.front();
        return back;
    }

    // Been here before. Arriving by a passage that was unmarked until now means
    // we have just closed a loop, so the rule is to turn round immediately
    // rather than press on and walk it again.
    if (back >= 0 && m_marks[m_pos.y()][m_pos.x()][back] == 1) {
        return back;
    }

    if (!unmarked.empty()) return unmarked.front();
    if (!markedOnce.empty()) return markedOnce.front();
    return -1; // every way out has been walked twice
}

void TremauxSolver::finish(MazeGrid &grid)
{
    for (const QPoint &cell : m_path.cells()) {
        grid[cell.y()][cell.x()].isSolution = true;
    }
    setPathLength(static_cast<int>(m_path.cells().size()));
    m_done = true;
}

bool TremauxSolver::step(MazeGrid &grid)
{
    if (m_done) {
        return false;
    }

    if (m_pos == m_end) {
        finish(grid);
        return false;
    }

    const int direction = chooseDirection(grid);
    m_seenCell[m_pos.y()][m_pos.x()] = true;

    if (direction < 0) {
        return false;
    }

    const QPoint next = m_pos + kDirections[direction];

    m_marks[m_pos.y()][m_pos.x()][direction] += 1;
    m_marks[next.y()][next.x()][opposite(direction)] += 1;

    m_pos = next;
    m_arrivalDir = direction;
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
