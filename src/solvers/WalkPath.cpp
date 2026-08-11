#include "solvers/WalkPath.h"

#include "MazeGrid.h"

void WalkPath::reset(const MazeGrid &grid, const QPoint &start)
{
    m_onPath.assign(grid.height(), std::vector<bool>(grid.width(), false));
    m_cells.clear();
    m_cells.push_back(start);
    m_onPath[start.y()][start.x()] = true;
}

void WalkPath::moveTo(const QPoint &p)
{
    if (m_onPath[p.y()][p.x()]) {
        // Already on the route, so everything walked since then was a detour.
        while (!m_cells.empty() && m_cells.back() != p) {
            const QPoint &back = m_cells.back();
            m_onPath[back.y()][back.x()] = false;
            m_cells.pop_back();
        }
        return;
    }

    m_cells.push_back(p);
    m_onPath[p.y()][p.x()] = true;
}
