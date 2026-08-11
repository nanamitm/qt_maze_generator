#include "generators/HuntAndKillGenerator.h"

#include "MazeGrid.h"

namespace {

// A cell counts as visited once it has been carved, so the grid itself is the
// visited set and no parallel bookkeeping is needed.
std::vector<QPoint> neighborsMatching(const MazeGrid &grid, const QPoint &p, bool wanted)
{
    std::vector<QPoint> result;
    for (const QPoint &n : grid.neighbors(p, 2)) {
        if (grid.isWall(n) == wanted) {
            result.push_back(n);
        }
    }
    return result;
}

} // namespace

void HuntAndKillGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();
    grid.carve(QPoint(1, 1));

    m_current = QPoint(1, 1);
    m_hunting = false;
    m_huntY = 1;
    m_done = false;
}

bool HuntAndKillGenerator::step(MazeGrid &grid)
{
    if (m_done) {
        return false;
    }

    if (!m_hunting) {
        const std::vector<QPoint> unvisited = neighborsMatching(grid, m_current, true);
        if (!unvisited.empty()) {
            std::uniform_int_distribution<size_t> dist(0, unvisited.size() - 1);
            const QPoint next = unvisited[dist(m_rng)];
            grid.carveBetween(m_current, next);
            m_current = next;

            grid.clearFrontier();
            grid[m_current.y()][m_current.x()].isFrontier = true;
            return true;
        }

        // Dead end: start hunting from the top. Restarting the scan every time
        // is what keeps this correct, since carving can expose an unvisited
        // cell in a row the previous hunt already passed.
        m_hunting = true;
        m_huntY = 1;
    }

    // One row per step, which is also what makes the scan visible.
    grid.clearFrontier();
    for (int x = 1; x <= grid.width() - 2; x += 2) {
        grid[m_huntY][x].isFrontier = true;

        const QPoint candidate(x, m_huntY);
        if (!grid.isWall(candidate)) {
            continue;
        }

        const std::vector<QPoint> visited = neighborsMatching(grid, candidate, false);
        if (visited.empty()) {
            continue;
        }

        std::uniform_int_distribution<size_t> dist(0, visited.size() - 1);
        grid.carveBetween(candidate, visited[dist(m_rng)]);
        m_current = candidate;
        m_hunting = false;
        return true;
    }

    m_huntY += 2;
    if (m_huntY > grid.height() - 2) {
        m_done = true;
        return false;
    }
    return true;
}
