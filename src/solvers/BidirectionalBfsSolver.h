#ifndef BIDIRECTIONALBFSSOLVER_H
#define BIDIRECTIONALBFSSOLVER_H

#include <QPoint>
#include <deque>
#include <vector>

#include "MazeSolver.h"

// Bidirectional breadth-first search. Runs one search out from the start and
// another out from the goal, one cell each per turn, and stops when they touch.
// Two searches of half the depth cover far less ground than one of full depth,
// so it visits noticeably fewer cells than plain BFS for the same answer.
class BidirectionalBfsSolver : public MazeSolver {
public:
    void init(MazeGrid &grid, const QPoint &start, const QPoint &end) override;
    bool step(MazeGrid &grid) override;

private:
    std::deque<QPoint> m_forwardQueue;
    std::deque<QPoint> m_backwardQueue;
    std::vector<std::vector<bool>> m_seenForward;
    std::vector<std::vector<bool>> m_seenBackward;
    // m_parent from the base class holds the forward tree; this is its mirror.
    std::vector<std::vector<QPoint>> m_parentBackward;
    bool m_expandForward = true;
};

#endif // BIDIRECTIONALBFSSOLVER_H
