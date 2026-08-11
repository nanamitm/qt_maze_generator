#include "generators/EllerGenerator.h"

#include <map>

#include "MazeGrid.h"

namespace {

// Cell column to grid coordinate.
int gridX(int column) { return 1 + column * 2; }

} // namespace

void EllerGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();

    m_setOf.assign(grid.cellsWide(), -1);
    m_nextSetId = 0;
    m_y = 1;
    m_column = 0;
    m_phase = Phase::StartRow;
}

bool EllerGenerator::isLastRow(const MazeGrid &grid) const
{
    return m_y + 2 > grid.height() - 2;
}

void EllerGenerator::relabel(int from, int to)
{
    for (int &id : m_setOf) {
        if (id == from) {
            id = to;
        }
    }
}

void EllerGenerator::beginRow(MazeGrid &grid)
{
    grid.clearFrontier();
    for (int c = 0; c < grid.cellsWide(); ++c) {
        grid.carve(QPoint(gridX(c), m_y));
        grid[m_y][gridX(c)].isFrontier = true;

        // Cells that did not come down from the row above start their own set.
        if (m_setOf[c] == -1) {
            m_setOf[c] = m_nextSetId++;
        }
    }
}

bool EllerGenerator::step(MazeGrid &grid)
{
    switch (m_phase) {
    case Phase::Done:
        return false;

    case Phase::StartRow:
        beginRow(grid);
        m_column = 0;
        m_phase = Phase::MergeAcross;
        return true;

    case Phase::MergeAcross: {
        if (m_column + 1 >= grid.cellsWide()) {
            m_phase = isLastRow(grid) ? Phase::Done : Phase::DropDown;
            return m_phase != Phase::Done;
        }

        const int left = m_setOf[m_column];
        const int right = m_setOf[m_column + 1];

        // On the last row every remaining set must be joined, otherwise parts
        // of the maze would never connect.
        std::uniform_int_distribution<int> coin(0, 1);
        const bool merge = (left != right) && (isLastRow(grid) || coin(m_rng) == 0);

        if (merge) {
            grid.carveBetween(QPoint(gridX(m_column), m_y), QPoint(gridX(m_column + 1), m_y));
            relabel(right, left);
        }

        ++m_column;
        return true;
    }

    case Phase::DropDown: {
        std::map<int, std::vector<int>> columnsBySet;
        for (int c = 0; c < grid.cellsWide(); ++c) {
            columnsBySet[m_setOf[c]].push_back(c);
        }

        std::vector<int> carried(grid.cellsWide(), -1);
        for (const auto &entry : columnsBySet) {
            const std::vector<int> &columns = entry.second;

            // Every set drops at least one cell, or it would be cut off from
            // the rest of the maze.
            std::uniform_int_distribution<size_t> pick(0, columns.size() - 1);
            const size_t guaranteed = pick(m_rng);

            std::uniform_int_distribution<int> coin(0, 1);
            for (size_t i = 0; i < columns.size(); ++i) {
                if (i != guaranteed && coin(m_rng) != 0) {
                    continue;
                }
                const int c = columns[i];
                grid.carveBetween(QPoint(gridX(c), m_y), QPoint(gridX(c), m_y + 2));
                carried[c] = entry.first;
            }
        }

        m_setOf = carried;
        m_y += 2;
        m_phase = Phase::StartRow;
        return true;
    }
    }

    return false;
}
