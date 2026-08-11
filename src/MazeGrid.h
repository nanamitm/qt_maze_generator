#ifndef MAZEGRID_H
#define MAZEGRID_H

#include <QPoint>
#include <vector>

struct Cell {
    bool isWall = true;
    bool isVisited = false;
    bool isFrontier = false;
    bool isSolution = false;
};

// The maze substrate, using the usual odd-coordinate convention: odd rows and
// columns are passages, even ones are the walls between them. Generators own no
// grid of their own, they carve into this one.
class MazeGrid {
public:
    void resize(int width, int height);

    int width() const { return m_width; }
    int height() const { return m_height; }

    // Row access, so callers can keep writing grid[row][col].
    std::vector<Cell>& operator[](int row) { return m_cells[row]; }
    const std::vector<Cell>& operator[](int row) const { return m_cells[row]; }

    bool isValid(int row, int col) const;
    bool isValid(const QPoint& p) const;

    bool isWall(const QPoint& p) const { return m_cells[p.y()][p.x()].isWall; }
    void carve(const QPoint& p) { m_cells[p.y()][p.x()].isWall = false; }

    // Carves both endpoints and the single cell between them. The two points
    // must be two steps apart on one axis, which is how every carving
    // generator advances.
    void carveBetween(const QPoint& a, const QPoint& b);

    void fillWithWalls();   // every cell becomes a wall
    void openWithBorder();  // interior open, outer ring walled

    void clearFrontier();
    void clearOverlays();   // visited, frontier and solution highlights

    // Neighbours `distance` away, excluding the outer ring.
    std::vector<QPoint> neighbors(const QPoint& p, int distance) const;
    // Adjacent cells that are not walls.
    std::vector<QPoint> walkableNeighbors(const QPoint& p) const;

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<std::vector<Cell>> m_cells;
};

#endif // MAZEGRID_H
