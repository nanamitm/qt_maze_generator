#include "generators/PrimGenerator.h"

#include "MazeGrid.h"

void PrimGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();
    grid.carve(QPoint(1, 1));

    m_candidates.clear();
    for (const QPoint &n : grid.neighbors(QPoint(1, 1), 2)) {
        m_candidates.push_back(n);
        grid[n.y()][n.x()].isFrontier = true;
    }
}

bool PrimGenerator::step(MazeGrid &grid)
{
    if (m_candidates.empty()) {
        return false;
    }

    std::uniform_int_distribution<size_t> dist(0, m_candidates.size() - 1);
    const size_t index = dist(m_rng);
    const QPoint next = m_candidates[index];
    m_candidates.erase(m_candidates.begin() + index);
    grid[next.y()][next.x()].isFrontier = false;

    if (grid.isWall(next)) {
        grid.carve(next);

        // Connect to a random neighbour that is already part of the maze.
        std::vector<QPoint> visited;
        for (const QPoint &n : grid.neighbors(next, 2)) {
            if (!grid.isWall(n)) {
                visited.push_back(n);
            }
        }
        if (!visited.empty()) {
            std::uniform_int_distribution<size_t> vDist(0, visited.size() - 1);
            grid.carveBetween(next, visited[vDist(m_rng)]);
        }

        for (const QPoint &n : grid.neighbors(next, 2)) {
            if (grid.isWall(n) && !grid[n.y()][n.x()].isFrontier) {
                m_candidates.push_back(n);
                grid[n.y()][n.x()].isFrontier = true;
            }
        }
    }

    return true;
}
