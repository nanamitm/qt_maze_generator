#ifndef HUNTANDKILLGENERATOR_H
#define HUNTANDKILLGENERATOR_H

#include <QPoint>

#include "MazeGenerator.h"

// Hunt-and-kill. Walks like the recursive backtracker but keeps no stack: when
// it runs into a dead end it scans the grid for an unvisited cell next to the
// maze and restarts from there. Same dead-end share as the backtracker, but
// far shorter longest paths, because a restart lands anywhere.
class HuntAndKillGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    QPoint m_current{1, 1};
    bool m_hunting = false;
    int m_huntY = 1;
    bool m_done = false;
};

#endif // HUNTANDKILLGENERATOR_H
