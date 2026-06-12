#include "MazeModel.h"
#include <algorithm>
#include <chrono>
#include <deque>

MazeModel::MazeModel(QObject *parent)
    : QObject(parent)
{
    // Seed the random generator
    m_rng.seed(std::chrono::system_clock::now().time_since_epoch().count());
    setSize(31, 31);
}

void MazeModel::setSize(int w, int h)
{
    // Grid dimensions must be odd to allow perfect wall-carving boundaries
    if (w % 2 == 0) w++;
    if (h % 2 == 0) h++;

    m_width = std::clamp(w, 5, 201);
    m_height = std::clamp(h, 5, 201);

    m_grid.assign(m_height, std::vector<Cell>(m_width));

    // Default positions
    m_startPos = QPoint(1, 1);
    m_endPos = QPoint(m_width - 2, m_height - 2);

    clearAll();
}

const Cell& MazeModel::cell(int row, int col) const
{
    static Cell dummy;
    if (isValid(row, col)) {
        return m_grid[row][col];
    }
    return dummy;
}

void MazeModel::setWall(int row, int col, bool isWall)
{
    if (isValid(row, col)) {
        // Start and End nodes can never be walls
        if (QPoint(col, row) == m_startPos || QPoint(col, row) == m_endPos) {
            return;
        }
        if (m_grid[row][col].isWall == isWall) {
            return;
        }
        clearPathOnly();
        m_grid[row][col].isWall = isWall;
        m_grid[row][col].isVisited = false;
        m_grid[row][col].isFrontier = false;
        m_grid[row][col].isSolution = false;
        emit gridChanged();
    }
}

void MazeModel::setStartPos(const QPoint& pos)
{
    if (isValid(pos) && !m_grid[pos.y()][pos.x()].isWall && pos != m_endPos) {
        clearPathOnly();
        m_startPos = pos;
        ensureEndpointConnections();
        emit gridChanged();
    }
}

void MazeModel::setEndPos(const QPoint& pos)
{
    if (isValid(pos) && !m_grid[pos.y()][pos.x()].isWall && pos != m_startPos) {
        clearPathOnly();
        m_endPos = pos;
        ensureEndpointConnections();
        emit gridChanged();
    }
}

void MazeModel::clearAll()
{
    m_generating = false;
    m_solving = false;
    m_finished = false;
    m_hasGeneratedMaze = false;

    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            m_grid[r][c].isWall = true;
            m_grid[r][c].isVisited = false;
            m_grid[r][c].isFrontier = false;
            m_grid[r][c].isSolution = false;
        }
    }

    // Carve start and end
    ensureEndpointConnections();

    emit gridChanged();
}

void MazeModel::clearPathOnly()
{
    m_solving = false;
    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            m_grid[r][c].isVisited = false;
            m_grid[r][c].isFrontier = false;
            m_grid[r][c].isSolution = false;
        }
    }
    emit gridChanged();
}

void MazeModel::resetToEmpty()
{
    m_generating = false;
    m_solving = false;
    m_finished = false;
    m_hasGeneratedMaze = false;

    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            bool isBorder = (r == 0 || r == m_height - 1 || c == 0 || c == m_width - 1);
            m_grid[r][c].isWall = isBorder;
            m_grid[r][c].isVisited = false;
            m_grid[r][c].isFrontier = false;
            m_grid[r][c].isSolution = false;
        }
    }

    ensureEndpointConnections();
    emit gridChanged();
}

// ----------------- Helpers -----------------

bool MazeModel::isValid(const QPoint& p) const
{
    return isValid(p.y(), p.x());
}

bool MazeModel::isValid(int r, int c) const
{
    return r >= 0 && r < m_height && c >= 0 && c < m_width;
}

std::vector<QPoint> MazeModel::getNeighbors(const QPoint& p, int distance) const
{
    std::vector<QPoint> neighbors;
    QPoint dirs[] = { QPoint(0, -distance), QPoint(0, distance), QPoint(-distance, 0), QPoint(distance, 0) };
    for (const auto& d : dirs) {
        QPoint n = p + d;
        // Keep inside bounds (excluding outer walls which are index 0 and width-1/height-1)
        if (n.x() > 0 && n.x() < m_width - 1 && n.y() > 0 && n.y() < m_height - 1) {
            neighbors.push_back(n);
        }
    }
    return neighbors;
}

std::vector<QPoint> MazeModel::getWalkableNeighbors(const QPoint& p) const
{
    std::vector<QPoint> neighbors;
    QPoint dirs[] = { QPoint(0, -1), QPoint(0, 1), QPoint(-1, 0), QPoint(1, 0) };
    for (const auto& d : dirs) {
        QPoint n = p + d;
        if (isValid(n) && !m_grid[n.y()][n.x()].isWall) {
            neighbors.push_back(n);
        }
    }
    return neighbors;
}

double MazeModel::heuristic(const QPoint& p1, const QPoint& p2) const
{
    return std::abs(p1.x() - p2.x()) + std::abs(p1.y() - p2.y());
}

QPoint MazeModel::nearestMazeAnchor(const QPoint& p) const
{
    int x = std::clamp(p.x(), 1, m_width - 2);
    int y = std::clamp(p.y(), 1, m_height - 2);

    if (x % 2 == 0) {
        if (x + 1 < m_width - 1) {
            ++x;
        } else {
            --x;
        }
    }
    if (y % 2 == 0) {
        if (y + 1 < m_height - 1) {
            ++y;
        } else {
            --y;
        }
    }

    return QPoint(x, y);
}

void MazeModel::carveEndpointConnection(const QPoint& p)
{
    if (!isValid(p)) return;

    QPoint anchor = nearestMazeAnchor(p);
    int x = p.x();
    int y = p.y();

    m_grid[y][x].isWall = false;
    while (x != anchor.x()) {
        x += (anchor.x() > x) ? 1 : -1;
        m_grid[y][x].isWall = false;
    }
    while (y != anchor.y()) {
        y += (anchor.y() > y) ? 1 : -1;
        m_grid[y][x].isWall = false;
    }
}

void MazeModel::ensureEndpointConnections()
{
    carveEndpointConnection(m_startPos);
    carveEndpointConnection(m_endPos);
}

void MazeModel::ensureEndReachable()
{
    ensureEndpointConnections();

    std::vector<std::vector<bool>> reachable(m_height, std::vector<bool>(m_width, false));
    std::deque<QPoint> queue;
    queue.push_back(m_startPos);
    reachable[m_startPos.y()][m_startPos.x()] = true;

    while (!queue.empty()) {
        QPoint curr = queue.front();
        queue.pop_front();

        for (const auto& next : getWalkableNeighbors(curr)) {
            if (!reachable[next.y()][next.x()]) {
                reachable[next.y()][next.x()] = true;
                queue.push_back(next);
            }
        }
    }

    if (reachable[m_endPos.y()][m_endPos.x()]) {
        return;
    }

    QPoint best = m_startPos;
    int bestDistance = std::abs(best.x() - m_endPos.x()) + std::abs(best.y() - m_endPos.y());
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (!reachable[y][x]) continue;
            int distance = std::abs(x - m_endPos.x()) + std::abs(y - m_endPos.y());
            if (distance < bestDistance) {
                bestDistance = distance;
                best = QPoint(x, y);
            }
        }
    }

    int x = m_endPos.x();
    int y = m_endPos.y();
    m_grid[y][x].isWall = false;

    while (x != best.x()) {
        x += (best.x() > x) ? 1 : -1;
        m_grid[y][x].isWall = false;
    }
    while (y != best.y()) {
        y += (best.y() > y) ? 1 : -1;
        m_grid[y][x].isWall = false;
    }
}

