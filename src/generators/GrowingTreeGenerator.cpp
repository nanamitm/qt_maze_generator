#include "generators/GrowingTreeGenerator.h"

#include "MazeGrid.h"

namespace {
constexpr int kNewestPercent = 75;
}

void GrowingTreeGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();
    grid.carve(QPoint(1, 1));

    m_active.clear();
    m_active.push_back(QPoint(1, 1));
}

size_t GrowingTreeGenerator::pickIndex()
{
    switch (m_policy) {
    case GrowingTreePolicy::Newest:
        return m_active.size() - 1;
    case GrowingTreePolicy::Oldest:
        return 0;
    case GrowingTreePolicy::Random: {
        std::uniform_int_distribution<size_t> dist(0, m_active.size() - 1);
        return dist(m_rng);
    }
    case GrowingTreePolicy::Mixed: {
        // The reference does not state its mix, so this was fitted to the
        // fingerprint it reports (15% dead ends). The dead-end share falls
        // steadily as the newest cell is favoured more: 20.4% at an even
        // split, 17.3% at 60%, 15.3% at 75%, 11.5% at 90%.
        std::uniform_int_distribution<int> roll(0, 99);
        if (roll(m_rng) < kNewestPercent) {
            return m_active.size() - 1;
        }
        std::uniform_int_distribution<size_t> dist(0, m_active.size() - 1);
        return dist(m_rng);
    }
    }
    return m_active.size() - 1;
}

bool GrowingTreeGenerator::step(MazeGrid &grid)
{
    if (m_active.empty()) {
        return false;
    }

    const size_t index = pickIndex();
    const QPoint curr = m_active[index];

    std::vector<QPoint> unvisited;
    for (const QPoint &n : grid.neighbors(curr, 2)) {
        if (grid.isWall(n)) {
            unvisited.push_back(n);
        }
    }

    if (unvisited.empty()) {
        // Nothing left to carve from here, so it stops being a candidate.
        m_active.erase(m_active.begin() + index);
        grid[curr.y()][curr.x()].isFrontier = false;
        return true;
    }

    std::uniform_int_distribution<size_t> dist(0, unvisited.size() - 1);
    const QPoint next = unvisited[dist(m_rng)];
    grid.carveBetween(curr, next);
    m_active.push_back(next);
    grid[next.y()][next.x()].isFrontier = true;

    return true;
}
