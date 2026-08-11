#include "generators/WilsonGenerator.h"

#include "MazeGrid.h"

void WilsonGenerator::init(MazeGrid &grid)
{
    grid.fillWithWalls();

    m_unvisited.clear();
    for (int y = 1; y <= grid.height() - 2; y += 2) {
        for (int x = 1; x <= grid.width() - 2; x += 2) {
            m_unvisited.push_back(QPoint(x, y));
        }
    }

    // One cell seeds the maze; every walk from here on aims at it.
    grid.carve(QPoint(1, 1));
    m_remaining = grid.cellCount() - 1;

    m_nextOf.assign(grid.cellCount(), QPoint(-1, -1));
    m_phase = Phase::PickStart;
}

bool WilsonGenerator::pickStart(MazeGrid &grid)
{
    while (!m_unvisited.empty()) {
        std::uniform_int_distribution<size_t> dist(0, m_unvisited.size() - 1);
        const size_t index = dist(m_rng);
        const QPoint candidate = m_unvisited[index];

        if (grid.isWall(candidate)) {
            m_walkStart = candidate;
            m_walkPos = candidate;
            return true;
        }

        // Already absorbed by an earlier walk.
        m_unvisited[index] = m_unvisited.back();
        m_unvisited.pop_back();
    }
    return false;
}

bool WilsonGenerator::step(MazeGrid &grid)
{
    if (m_remaining <= 0) {
        return false;
    }

    switch (m_phase) {
    case Phase::PickStart:
        if (!pickStart(grid)) {
            return false;
        }
        m_phase = Phase::Walk;
        return true;

    case Phase::Walk: {
        const std::vector<QPoint> options = grid.neighbors(m_walkPos, 2);
        if (options.empty()) {
            return false;
        }

        std::uniform_int_distribution<size_t> dist(0, options.size() - 1);
        const QPoint next = options[dist(m_rng)];

        // Overwriting the recorded step is what erases loops: if the walk
        // comes back here later, the earlier detour is simply forgotten.
        m_nextOf[grid.cellIndex(m_walkPos)] = next;

        grid.clearFrontier();
        grid[next.y()][next.x()].isFrontier = true;

        if (!grid.isWall(next)) {
            m_retracePos = m_walkStart;
            m_phase = Phase::Retrace;
        } else {
            m_walkPos = next;
        }
        return true;
    }

    case Phase::Retrace: {
        const QPoint next = m_nextOf[grid.cellIndex(m_retracePos)];
        const bool reachedMaze = !grid.isWall(next);

        grid.carveBetween(m_retracePos, next);
        --m_remaining;

        m_retracePos = next;
        if (reachedMaze) {
            m_phase = Phase::PickStart;
        }
        return true;
    }
    }

    return false;
}
