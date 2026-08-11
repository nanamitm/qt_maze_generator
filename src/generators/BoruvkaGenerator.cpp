#include "generators/BoruvkaGenerator.h"

#include <map>

#include "MazeGrid.h"

void BoruvkaGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();

    for (int y = 1; y <= grid.height() - 2; y += 2) {
        for (int x = 1; x <= grid.width() - 2; x += 2) {
            grid.carve(QPoint(x, y));
        }
    }

    m_round.clear();
    m_next = 0;
    m_sets.reset(grid.cellCount());
}

bool BoruvkaGenerator::startRound(MazeGrid &grid)
{
    // Collect every wall that leaves its group, keyed by the group it leaves.
    std::map<int, std::vector<std::pair<QPoint, QPoint>>> outgoing;

    for (int y = 1; y <= grid.height() - 2; y += 2) {
        for (int x = 1; x <= grid.width() - 2; x += 2) {
            const QPoint from(x, y);
            const int fromRoot = m_sets.find(grid.cellIndex(from));

            for (const QPoint &to : grid.neighbors(from, 2)) {
                if (m_sets.find(grid.cellIndex(to)) != fromRoot) {
                    outgoing[fromRoot].emplace_back(from, to);
                }
            }
        }
    }

    if (outgoing.empty()) {
        return false;
    }

    // One pick per group, at random rather than by weight, since the walls
    // here carry no cost.
    m_round.clear();
    for (const auto &entry : outgoing) {
        const auto &candidates = entry.second;
        std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
        m_round.push_back(candidates[dist(m_rng)]);
    }
    m_next = 0;
    return true;
}

bool BoruvkaGenerator::step(MazeGrid &grid)
{
    while (true) {
        while (m_next < m_round.size()) {
            const auto edge = m_round[m_next++];
            // Two groups can pick each other in the same round, so the second
            // of the pair is already redundant by the time it is applied.
            if (!m_sets.unite(grid.cellIndex(edge.first), grid.cellIndex(edge.second))) {
                continue;
            }

            grid.carveBetween(edge.first, edge.second);

            grid.clearFrontier();
            grid[edge.first.y()][edge.first.x()].isFrontier = true;
            grid[edge.second.y()][edge.second.x()].isFrontier = true;
            return true;
        }

        if (!startRound(grid)) {
            return false;
        }
    }
}
