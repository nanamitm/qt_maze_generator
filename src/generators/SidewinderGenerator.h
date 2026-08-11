#ifndef SIDEWINDERGENERATOR_H
#define SIDEWINDERGENERATOR_H

#include "MazeGenerator.h"

// Sidewinder. Scans row by row building a horizontal run, then closes the run
// by carving north from one cell of it. Only the top row keeps the binary
// tree's corridor artefact, because it has nowhere north to go.
class SidewinderGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    int m_x = 1;
    int m_y = 1;
    int m_runStart = 1;
};

#endif // SIDEWINDERGENERATOR_H
