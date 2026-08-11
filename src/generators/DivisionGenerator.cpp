#include "generators/DivisionGenerator.h"

#include "MazeGrid.h"

void DivisionGenerator::init(MazeGrid &grid)
{
    grid.openWithBorder();

    m_chambers.clear();
    m_chambers.push_back(Chamber{1, grid.width() - 2, 1, grid.height() - 2});
}

bool DivisionGenerator::step(MazeGrid &grid)
{
    if (m_chambers.empty()) {
        return false;
    }

    const Chamber chamber = m_chambers.back();
    m_chambers.pop_back();

    const int w = chamber.xEnd - chamber.xStart;
    const int h = chamber.yEnd - chamber.yStart;

    if (w < 2 || h < 2) {
        return true; // too small to split, carry on with the next chamber
    }

    bool horizontal = false;
    if (w > h) {
        horizontal = false;
    } else if (h > w) {
        horizontal = true;
    } else {
        std::uniform_int_distribution<int> coin(0, 1);
        horizontal = (coin(m_rng) == 0);
    }

    if (horizontal) {
        // Even y coordinate for the wall, inside [yStart + 1, yEnd - 1].
        const int yRange = (chamber.yEnd - 1) - (chamber.yStart + 1);
        if (yRange < 0) return true;
        std::uniform_int_distribution<int> yDist(0, yRange / 2);
        const int wy = chamber.yStart + 1 + 2 * yDist(m_rng);

        // Odd x coordinate for the gap, inside [xStart, xEnd].
        std::uniform_int_distribution<int> xDist(0, (chamber.xEnd - chamber.xStart) / 2);
        const int px = chamber.xStart + 2 * xDist(m_rng);

        for (int col = chamber.xStart - 1; col <= chamber.xEnd + 1; ++col) {
            if (col != px) {
                grid[wy][col].isWall = true;
            }
        }

        m_chambers.push_back(Chamber{chamber.xStart, chamber.xEnd, chamber.yStart, wy - 1});
        m_chambers.push_back(Chamber{chamber.xStart, chamber.xEnd, wy + 1, chamber.yEnd});
    } else {
        // Even x coordinate for the wall, inside [xStart + 1, xEnd - 1].
        const int xRange = (chamber.xEnd - 1) - (chamber.xStart + 1);
        if (xRange < 0) return true;
        std::uniform_int_distribution<int> xDist(0, xRange / 2);
        const int wx = chamber.xStart + 1 + 2 * xDist(m_rng);

        // Odd y coordinate for the gap, inside [yStart, yEnd].
        std::uniform_int_distribution<int> yDist(0, (chamber.yEnd - chamber.yStart) / 2);
        const int py = chamber.yStart + 2 * yDist(m_rng);

        for (int r = chamber.yStart - 1; r <= chamber.yEnd + 1; ++r) {
            if (r != py) {
                grid[r][wx].isWall = true;
            }
        }

        m_chambers.push_back(Chamber{chamber.xStart, wx - 1, chamber.yStart, chamber.yEnd});
        m_chambers.push_back(Chamber{wx + 1, chamber.xEnd, chamber.yStart, chamber.yEnd});
    }

    return true;
}
