#ifndef DIVISIONGENERATOR_H
#define DIVISIONGENERATOR_H

#include <vector>

#include "MazeGenerator.h"

// Recursive division. Starts from an open room and repeatedly splits chambers
// with a wall that has exactly one gap in it.
class DivisionGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    struct Chamber {
        int xStart, xEnd; // odd coordinates
        int yStart, yEnd; // odd coordinates
    };

    std::vector<Chamber> m_chambers;
};

#endif // DIVISIONGENERATOR_H
