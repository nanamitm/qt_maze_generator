#ifndef WALKPATH_H
#define WALKPATH_H

#include <QPoint>
#include <vector>

class MazeGrid;

// The simple route from the start to wherever a walking solver currently
// stands. Walkers retrace their steps constantly, so their raw trail is not a
// route; stepping onto a cell that is already on the route rewinds to it,
// which drops the whole excursion and leaves a route with no detour in it.
class WalkPath {
public:
    void reset(const MazeGrid &grid, const QPoint &start);
    void moveTo(const QPoint &p);

    const std::vector<QPoint>& cells() const { return m_cells; }

private:
    std::vector<QPoint> m_cells;
    std::vector<std::vector<bool>> m_onPath;
};

#endif // WALKPATH_H
