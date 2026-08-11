#include "solvers/BidirectionalBfsSolver.h"

#include "MazeGrid.h"

void BidirectionalBfsSolver::init(MazeGrid &grid, const QPoint &start, const QPoint &end)
{
    beginSearch(grid, start, end);

    m_seenForward.assign(grid.height(), std::vector<bool>(grid.width(), false));
    m_seenBackward.assign(grid.height(), std::vector<bool>(grid.width(), false));
    m_parentBackward.assign(grid.height(), std::vector<QPoint>(grid.width(), QPoint(-1, -1)));

    m_forwardQueue.clear();
    m_forwardQueue.push_back(start);
    m_seenForward[start.y()][start.x()] = true;

    m_backwardQueue.clear();
    m_backwardQueue.push_back(end);
    m_seenBackward[end.y()][end.x()] = true;
    grid[end.y()][end.x()].isFrontier = true;

    m_expandForward = true;
}

bool BidirectionalBfsSolver::step(MazeGrid &grid)
{
    // If either side runs dry there is no route between them.
    if (m_forwardQueue.empty() || m_backwardQueue.empty()) {
        return false;
    }

    // Strict alternation, so the two wavefronts stay the same size.
    const bool forward = m_expandForward;
    m_expandForward = !m_expandForward;

    std::deque<QPoint> &queue = forward ? m_forwardQueue : m_backwardQueue;
    std::vector<std::vector<bool>> &seen = forward ? m_seenForward : m_seenBackward;
    const std::vector<std::vector<bool>> &otherSeen = forward ? m_seenBackward : m_seenForward;
    std::vector<std::vector<QPoint>> &parents = forward ? m_parent : m_parentBackward;

    const QPoint curr = queue.front();
    queue.pop_front();

    grid[curr.y()][curr.x()].isFrontier = false;
    grid[curr.y()][curr.x()].isVisited = true;

    if (otherSeen[curr.y()][curr.x()]) {
        // The two halves meet here. Painting from the meeting point along both
        // parent trees gives the whole route; the meeting cell belongs to both,
        // so it is counted twice.
        const int toStart = paintChain(grid, m_parent, curr);
        const int toEnd = paintChain(grid, m_parentBackward, curr);
        setPathLength(toStart + toEnd - 1);
        return false;
    }

    for (const QPoint &next : grid.walkableNeighbors(curr)) {
        if (seen[next.y()][next.x()]) {
            continue;
        }
        seen[next.y()][next.x()] = true;
        parents[next.y()][next.x()] = curr;
        grid[next.y()][next.x()].isFrontier = true;
        queue.push_back(next);
    }

    return true;
}
