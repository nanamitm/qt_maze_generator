#ifndef BINARYTREEGENERATOR_H
#define BINARYTREEGENERATOR_H

#include "MazeGenerator.h"

// Binary tree. Visits every cell once and carves either north or east, so the
// maze needs no state beyond the scan position. The trade-off is a strong
// diagonal bias: the top row and the right column are always single corridors.
class BinaryTreeGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    int m_x = 1;
    int m_y = 1;
};

#endif // BINARYTREEGENERATOR_H
