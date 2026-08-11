#ifndef KRUSKALGENERATOR_H
#define KRUSKALGENERATOR_H

#include <QPoint>
#include <utility>
#include <vector>

#include "MazeGenerator.h"
#include "generators/DisjointSet.h"

// Randomized Kruskal. Shuffles every wall that could be opened and opens the
// ones that join two groups that were not connected yet. Unlike the walking
// algorithms, it grows all over the grid at once.
class KruskalGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    std::vector<std::pair<QPoint, QPoint>> m_edges;
    size_t m_next = 0;
    DisjointSet m_sets;
};

#endif // KRUSKALGENERATOR_H
