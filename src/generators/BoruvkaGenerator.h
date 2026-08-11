#ifndef BORUVKAGENERATOR_H
#define BORUVKAGENERATOR_H

#include <QPoint>
#include <utility>
#include <vector>

#include "MazeGenerator.h"
#include "generators/DisjointSet.h"

// Randomized Boruvka. Works in rounds: every group picks one wall out to a
// different group, then all of those are opened at once, roughly halving the
// number of groups each round. Kruskal considers one wall at a time, this
// considers every group at a time.
class BoruvkaGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    // Returns false when no group has an outgoing wall left, which means the
    // maze is a single group and therefore finished.
    bool startRound(MazeGrid &grid);

    std::vector<std::pair<QPoint, QPoint>> m_round;
    size_t m_next = 0;
    DisjointSet m_sets;
};

#endif // BORUVKAGENERATOR_H
