#ifndef WILSONGENERATOR_H
#define WILSONGENERATOR_H

#include <QPoint>
#include <vector>

#include "MazeGenerator.h"

// Wilson's algorithm, loop-erased random walk. Walks from a cell outside the
// maze until it reaches the maze, then keeps only the walk's final route: any
// loop it made along the way is erased by the act of overwriting the recorded
// direction. Like Aldous-Broder it picks every possible maze with equal
// probability, but it gets faster as the maze grows rather than slower.
class WilsonGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

    int stepScale() const override { return 24; }

private:
    enum class Phase {
        PickStart,
        Walk,
        Retrace
    };

    bool pickStart(MazeGrid &grid);

    Phase m_phase = Phase::PickStart;
    QPoint m_walkStart{1, 1};
    QPoint m_walkPos{1, 1};
    QPoint m_retracePos{1, 1};
    std::vector<QPoint> m_nextOf;    // per cell, the step the walk took from it
    std::vector<QPoint> m_unvisited; // candidate walk starts, pruned lazily
    int m_remaining = 0;
};

#endif // WILSONGENERATOR_H
