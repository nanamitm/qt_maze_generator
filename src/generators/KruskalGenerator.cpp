#include "generators/KruskalGenerator.h"

#include <algorithm>

#include "MazeGrid.h"

void KruskalGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();

    // Every cell starts as its own group, so they are all open from the start
    // and only the walls between them are in question.
    for (int y = 1; y <= grid.height() - 2; y += 2) {
        for (int x = 1; x <= grid.width() - 2; x += 2) {
            grid.carve(QPoint(x, y));
        }
    }

    m_edges.clear();
    for (int y = 1; y <= grid.height() - 2; y += 2) {
        for (int x = 1; x <= grid.width() - 2; x += 2) {
            const QPoint from(x, y);
            if (x + 2 <= grid.width() - 2) {
                m_edges.emplace_back(from, QPoint(x + 2, y));
            }
            if (y + 2 <= grid.height() - 2) {
                m_edges.emplace_back(from, QPoint(x, y + 2));
            }
        }
    }
    std::shuffle(m_edges.begin(), m_edges.end(), m_rng);

    m_next = 0;
    m_sets.reset(grid.cellCount());
}

bool KruskalGenerator::step(MazeGrid &grid)
{
    // Skip past edges that would close a loop rather than spending a frame on
    // each of them; roughly half of all edges are rejected.
    while (m_next < m_edges.size()) {
        const auto edge = m_edges[m_next++];
        if (!m_sets.unite(grid.cellIndex(edge.first), grid.cellIndex(edge.second))) {
            continue;
        }

        grid.carveBetween(edge.first, edge.second);

        grid.clearFrontier();
        grid[edge.first.y()][edge.first.x()].isFrontier = true;
        grid[edge.second.y()][edge.second.x()].isFrontier = true;
        return true;
    }

    return false;
}
