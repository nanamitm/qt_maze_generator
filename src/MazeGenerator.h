#ifndef MAZEGENERATOR_H
#define MAZEGENERATOR_H

#include <random>

class MazeGrid;

// A generator is a resumable state machine: init() prepares the grid and the
// algorithm state, step() performs one visible unit of work and returns false
// once the maze is complete. Animated generation drives it from a timer and
// instant generation spins on it, so each algorithm is written only once.
class MazeGenerator {
public:
    explicit MazeGenerator(std::mt19937 &rng) : m_rng(rng) {}
    virtual ~MazeGenerator() = default;

    MazeGenerator(const MazeGenerator&) = delete;
    MazeGenerator& operator=(const MazeGenerator&) = delete;

    virtual void init(MazeGrid &grid) = 0;
    virtual bool step(MazeGrid &grid) = 0;

    // Multiplier applied to the animation step budget, for algorithms whose
    // single step is too small a change to be worth a frame of its own.
    virtual int stepScale() const { return 1; }

protected:
    std::mt19937 &m_rng;
};

#endif // MAZEGENERATOR_H
