#ifndef ALDOUSBRODERGENERATOR_H
#define ALDOUSBRODERGENERATOR_H

#include <QPoint>

#include "MazeGenerator.h"

// Aldous-Broder. Wanders at random and carves whenever it happens onto a cell
// it has not seen, which picks every possible maze with equal probability. The
// price is the cover time of a random walk: it spends most of its run crossing
// ground it has already visited.
class AldousBroderGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

    int stepScale() const override { return 24; }

private:
    QPoint m_current{1, 1};
    int m_remaining = 0;
};

#endif // ALDOUSBRODERGENERATOR_H
