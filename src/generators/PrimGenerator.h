#ifndef PRIMGENERATOR_H
#define PRIMGENERATOR_H

#include <QPoint>
#include <vector>

#include "MazeGenerator.h"

// Randomized Prim's algorithm. Keeps a frontier of cells adjacent to the maze
// and grows by picking one of them at random.
class PrimGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    std::vector<QPoint> m_candidates;
};

#endif // PRIMGENERATOR_H
