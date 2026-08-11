#include "generators/DfsGenerator.h"

#include "MazeGrid.h"

void DfsGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();
    grid.carve(QPoint(1, 1));

    m_stack.clear();
    m_stack.push_back(QPoint(1, 1));
}

bool DfsGenerator::step(MazeGrid &grid)
{
    if (m_stack.empty()) {
        return false;
    }

    grid.clearFrontier();

    const QPoint curr = m_stack.back();
    grid[curr.y()][curr.x()].isFrontier = true; // highlight the current head

    std::vector<QPoint> unvisited;
    for (const QPoint &n : grid.neighbors(curr, 2)) {
        if (grid.isWall(n)) {
            unvisited.push_back(n);
        }
    }

    if (!unvisited.empty()) {
        std::uniform_int_distribution<size_t> dist(0, unvisited.size() - 1);
        const QPoint next = unvisited[dist(m_rng)];

        grid.carveBetween(curr, next);
        m_stack.push_back(next);
        grid[next.y()][next.x()].isFrontier = true;
    } else {
        m_stack.pop_back();
        if (!m_stack.empty()) {
            const QPoint head = m_stack.back();
            grid[head.y()][head.x()].isFrontier = true;
        }
    }

    return true;
}
