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

MazeModel::~MazeModel() = default;

void MazeModel::setSize(int w, int h)
{
    // Grid dimensions must be odd to allow perfect wall-carving boundaries
    if (w % 2 == 0) w++;
    if (h % 2 == 0) h++;

    m_grid.resize(std::clamp(w, 5, 201), std::clamp(h, 5, 201));

    // Default positions
    m_startPos = QPoint(1, 1);
    m_endPos = QPoint(m_grid.width() - 2, m_grid.height() - 2);

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
        m_grid[row][col] = Cell{};
        m_grid[row][col].isWall = isWall;
        emit gridChanged();
    }
}

void MazeModel::setStartPos(const QPoint& pos)
{
    if (isValid(pos) && !m_grid.isWall(pos) && pos != m_endPos) {
        clearPathOnly();
        m_startPos = pos;
        ensureEndpointConnections();
        emit gridChanged();
    }
}

void MazeModel::setEndPos(const QPoint& pos)
{
    if (isValid(pos) && !m_grid.isWall(pos) && pos != m_startPos) {
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

    m_grid.fillWithWalls();

    // Carve start and end
    ensureEndpointConnections();

    emit gridChanged();
}

void MazeModel::clearPathOnly()
{
    m_solving = false;
    m_grid.clearOverlays();
    emit gridChanged();
}

void MazeModel::resetToEmpty()
{
    m_generating = false;
    m_solving = false;
    m_finished = false;
    m_hasGeneratedMaze = false;

    m_grid.openWithBorder();

    ensureEndpointConnections();
    emit gridChanged();
}

// ----------------- Helpers -----------------

bool MazeModel::isValid(const QPoint& p) const
{
    return m_grid.isValid(p);
}

bool MazeModel::isValid(int r, int c) const
{
    return m_grid.isValid(r, c);
}

double MazeModel::heuristic(const QPoint& p1, const QPoint& p2) const
{
    return std::abs(p1.x() - p2.x()) + std::abs(p1.y() - p2.y());
}

QPoint MazeModel::nearestMazeAnchor(const QPoint& p) const
{
    int x = std::clamp(p.x(), 1, m_grid.width() - 2);
    int y = std::clamp(p.y(), 1, m_grid.height() - 2);

    if (x % 2 == 0) {
        if (x + 1 < m_grid.width() - 1) {
            ++x;
        } else {
            --x;
        }
    }
    if (y % 2 == 0) {
        if (y + 1 < m_grid.height() - 1) {
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

    const QPoint anchor = nearestMazeAnchor(p);
    int x = p.x();
    int y = p.y();

    m_grid.carve(QPoint(x, y));
    while (x != anchor.x()) {
        x += (anchor.x() > x) ? 1 : -1;
        m_grid.carve(QPoint(x, y));
    }
    while (y != anchor.y()) {
        y += (anchor.y() > y) ? 1 : -1;
        m_grid.carve(QPoint(x, y));
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

    std::vector<std::vector<bool>> reachable(m_grid.height(), std::vector<bool>(m_grid.width(), false));
    std::deque<QPoint> queue;
    queue.push_back(m_startPos);
    reachable[m_startPos.y()][m_startPos.x()] = true;

    while (!queue.empty()) {
        const QPoint curr = queue.front();
        queue.pop_front();

        for (const auto& next : m_grid.walkableNeighbors(curr)) {
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
    for (int y = 0; y < m_grid.height(); ++y) {
        for (int x = 0; x < m_grid.width(); ++x) {
            if (!reachable[y][x]) continue;
            const int distance = std::abs(x - m_endPos.x()) + std::abs(y - m_endPos.y());
            if (distance < bestDistance) {
                bestDistance = distance;
                best = QPoint(x, y);
            }
        }
    }

    int x = m_endPos.x();
    int y = m_endPos.y();
    m_grid.carve(QPoint(x, y));

    while (x != best.x()) {
        x += (best.x() > x) ? 1 : -1;
        m_grid.carve(QPoint(x, y));
    }
    while (y != best.y()) {
        y += (best.y() > y) ? 1 : -1;
        m_grid.carve(QPoint(x, y));
    }
}
