#ifndef BESTFIRST_H
#define BESTFIRST_H

#include <QPoint>

#include <cmath>

// Shared by the solvers that expand cells in order of a score: whichever cell
// scores lowest comes out of the queue first. What goes into that score is the
// difference between them.
struct BestFirstNode {
    QPoint pos;
    double score;
    bool operator>(const BestFirstNode& other) const {
        return score > other.score;
    }
};

// Manhattan distance. On a four-way grid it never overestimates the distance
// left, which is the property A* needs to stay optimal.
inline double manhattan(const QPoint& a, const QPoint& b)
{
    return std::abs(a.x() - b.x()) + std::abs(a.y() - b.y());
}

#endif // BESTFIRST_H
