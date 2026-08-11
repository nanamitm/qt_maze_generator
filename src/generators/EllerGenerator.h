#ifndef ELLERGENERATOR_H
#define ELLERGENERATOR_H

#include <vector>

#include "MazeGenerator.h"

// Eller's algorithm. Works one row at a time and never looks back, so it only
// ever holds the set membership of the current row: the one algorithm here
// that could generate a maze taller than memory.
class EllerGenerator : public MazeGenerator {
public:
    using MazeGenerator::MazeGenerator;

    void init(MazeGrid &grid) override;
    bool step(MazeGrid &grid) override;

private:
    enum class Phase {
        StartRow,
        MergeAcross, // decide the horizontal joins, one wall per step
        DropDown,    // carry each set into the next row
        Done
    };

    void beginRow(MazeGrid &grid);
    void relabel(int from, int to);
    bool isLastRow(const MazeGrid &grid) const;

    Phase m_phase = Phase::StartRow;
    int m_y = 1;
    int m_column = 0;          // cell column being considered
    int m_nextSetId = 0;
    std::vector<int> m_setOf;  // set id per cell column, current row only
};

#endif // ELLERGENERATOR_H
