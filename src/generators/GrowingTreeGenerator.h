#ifndef GROWINGTREEGENERATOR_H
#define GROWINGTREEGENERATOR_H

#include <QPoint>
#include <vector>

#include "MazeGenerator.h"

// Growing tree. Keeps a list of cells still worth extending and carves from
// one of them each step; which one it picks is the whole algorithm. Picking
// the newest reproduces the recursive backtracker, picking at random
// reproduces Prim, and picking the oldest sweeps outwards in a wavefront that
// leaves almost no dead ends.
enum class GrowingTreePolicy {
    Newest,
    Oldest,
    Random,
    Mixed // half newest, half random
};

class GrowingTreeGenerator : public MazeGenerator {
public:
    GrowingTreeGenerator(std::mt19937 &rng, GrowingTreePolicy policy)
        : MazeGenerator(rng), m_policy(policy) {}

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    size_t pickIndex();

    GrowingTreePolicy m_policy;
    std::vector<QPoint> m_active;
};

#endif // GROWINGTREEGENERATOR_H
