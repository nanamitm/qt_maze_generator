#include "generators/SidewinderGenerator.h"

#include "MazeGrid.h"

void SidewinderGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();
    m_x = 1;
    m_y = 1;
    m_runStart = 1;
}

bool SidewinderGenerator::step(MazeGrid &grid)
{
    if (m_y > grid.height() - 2) {
        return false;
    }

    const QPoint curr(m_x, m_y);
    grid.carve(curr);

    grid.clearFrontier();
    grid[curr.y()][curr.x()].isFrontier = true;

    const bool atEastEdge = (m_x + 2 > grid.width() - 2);
    const bool atTopRow = (m_y == 1);

    // The top row has nowhere north to close out into, so it runs east to the
    // wall and becomes a single corridor.
    std::uniform_int_distribution<int> coin(0, 1);
    const bool closeOut = atEastEdge || (!atTopRow && coin(m_rng) == 0);

    if (closeOut) {
        if (!atTopRow) {
            // Carve north from a random cell of the finished run.
            const int runCells = (m_x - m_runStart) / 2 + 1;
            std::uniform_int_distribution<int> pick(0, runCells - 1);
            const int cx = m_runStart + 2 * pick(m_rng);
            grid.carveBetween(QPoint(cx, m_y), QPoint(cx, m_y - 2));
        }
        m_runStart = m_x + 2;
    } else {
        grid.carveBetween(curr, QPoint(m_x + 2, m_y));
    }

    m_x += 2;
    if (m_x > grid.width() - 2) {
        m_x = 1;
        m_y += 2;
        m_runStart = 1;
    }

    return true;
}
