#include "generators/BinaryTreeGenerator.h"

#include "MazeGrid.h"

void BinaryTreeGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();
    m_x = 1;
    m_y = 1;
}

bool BinaryTreeGenerator::step(MazeGrid &grid)
{
    if (m_y > grid.height() - 2) {
        return false;
    }

    const QPoint curr(m_x, m_y);
    grid.carve(curr);

    grid.clearFrontier();
    grid[curr.y()][curr.x()].isFrontier = true;

    // North and east, so the last row carved in either direction closes the
    // maze off along the top and the right edge.
    std::vector<QPoint> options;
    if (m_y - 2 >= 1) {
        options.push_back(QPoint(m_x, m_y - 2));
    }
    if (m_x + 2 <= grid.width() - 2) {
        options.push_back(QPoint(m_x + 2, m_y));
    }

    if (!options.empty()) {
        std::uniform_int_distribution<size_t> dist(0, options.size() - 1);
        grid.carveBetween(curr, options[dist(m_rng)]);
    }

    m_x += 2;
    if (m_x > grid.width() - 2) {
        m_x = 1;
        m_y += 2;
    }

    return true;
}
