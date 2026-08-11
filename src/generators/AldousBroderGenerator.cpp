#include "generators/AldousBroderGenerator.h"

#include "MazeGrid.h"

void AldousBroderGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();

    m_current = QPoint(1, 1);
    grid.carve(m_current);
    m_remaining = grid.cellCount() - 1;
}

bool AldousBroderGenerator::step(MazeGrid &grid)
{
    if (m_remaining <= 0) {
        return false;
    }

    const std::vector<QPoint> options = grid.neighbors(m_current, 2);
    if (options.empty()) {
        return false;
    }

    std::uniform_int_distribution<size_t> dist(0, options.size() - 1);
    const QPoint next = options[dist(m_rng)];

    if (grid.isWall(next)) {
        grid.carveBetween(m_current, next);
        --m_remaining;
    }

    m_current = next;

    grid.clearFrontier();
    grid[m_current.y()][m_current.x()].isFrontier = true;
    return true;
}
