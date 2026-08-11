#include "MazeGrid.h"

void MazeGrid::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    m_cells.assign(m_height, std::vector<Cell>(m_width));
}

bool MazeGrid::isValid(int row, int col) const
{
    return row >= 0 && row < m_height && col >= 0 && col < m_width;
}

bool MazeGrid::isValid(const QPoint& p) const
{
    return isValid(p.y(), p.x());
}

void MazeGrid::carveBetween(const QPoint& a, const QPoint& b)
{
    carve(a);
    carve(b);
    carve(QPoint(a.x() + (b.x() - a.x()) / 2, a.y() + (b.y() - a.y()) / 2));
}

void MazeGrid::fillWithWalls()
{
    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            m_cells[r][c] = Cell{};
        }
    }
}

void MazeGrid::openWithBorder()
{
    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            const bool isBorder = (r == 0 || r == m_height - 1 || c == 0 || c == m_width - 1);
            m_cells[r][c] = Cell{};
            m_cells[r][c].isWall = isBorder;
        }
    }
}

void MazeGrid::clearFrontier()
{
    for (auto& row : m_cells) {
        for (auto& cell : row) {
            cell.isFrontier = false;
        }
    }
}

void MazeGrid::clearOverlays()
{
    for (auto& row : m_cells) {
        for (auto& cell : row) {
            cell.isVisited = false;
            cell.isFrontier = false;
            cell.isSolution = false;
        }
    }
}

std::vector<QPoint> MazeGrid::neighbors(const QPoint& p, int distance) const
{
    std::vector<QPoint> result;
    const QPoint dirs[] = {
        QPoint(0, -distance), QPoint(0, distance),
        QPoint(-distance, 0), QPoint(distance, 0)
    };

    for (const auto& d : dirs) {
        const QPoint n = p + d;
        // Keep inside the outer walls, which are always index 0 and size - 1.
        if (n.x() > 0 && n.x() < m_width - 1 && n.y() > 0 && n.y() < m_height - 1) {
            result.push_back(n);
        }
    }
    return result;
}

std::vector<QPoint> MazeGrid::walkableNeighbors(const QPoint& p) const
{
    std::vector<QPoint> result;
    const QPoint dirs[] = { QPoint(0, -1), QPoint(0, 1), QPoint(-1, 0), QPoint(1, 0) };

    for (const auto& d : dirs) {
        const QPoint n = p + d;
        if (isValid(n) && !m_cells[n.y()][n.x()].isWall) {
            result.push_back(n);
        }
    }
    return result;
}
