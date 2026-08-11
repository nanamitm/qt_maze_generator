#ifndef DFSGENERATOR_H
#define DFSGENERATOR_H

#include <QPoint>
#include <vector>

#include "MazeGenerator.h"

// Recursive backtracker. Carves a passage to a random unvisited neighbour and
// backtracks through an explicit stack when it runs into a dead end.
class DfsGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    std::vector<QPoint> m_stack;
};

#endif // DFSGENERATOR_H
