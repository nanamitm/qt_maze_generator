#include "MazeModel.h"

#include <algorithm>


// ----------------- Generation -----------------

void MazeModel::initGeneration(GeneratorType type)
{
    m_activeGenType = type;
    m_hasGeneratedMaze = false;

    if (type == GeneratorType::Division) {
        resetToEmpty();
        m_divisionStack.clear();
        m_divisionStack.push_back(DivisionChamber{1, m_width - 2, 1, m_height - 2});
    } else {
        clearAll();
        // DFS and Prim starts at (1, 1)
        m_grid[1][1].isWall = false;

        if (type == GeneratorType::DFS) {
            m_genStack.clear();
            m_genStack.push_back(QPoint(1, 1));
        } else if (type == GeneratorType::Prim) {
            m_primCandidates.clear();
            std::vector<QPoint> initNeighbors = getNeighbors(QPoint(1, 1), 2);
            for (const auto& n : initNeighbors) {
                m_primCandidates.push_back(n);
                m_grid[n.y()][n.x()].isFrontier = true;
            }
        }
    }

    m_generating = true;
    m_solving = false;
    m_finished = false;

    emit gridChanged();
    emit statusUpdated("Generating...");
}

bool MazeModel::stepGeneration()
{
    if (!m_generating) return false;

    if (m_activeGenType == GeneratorType::DFS) {
        if (m_genStack.empty()) {
            m_generating = false;
            m_finished = true;
            m_hasGeneratedMaze = true;
            // Clear frontier colors
            for (int r = 0; r < m_height; ++r) {
                for (int c = 0; c < m_width; ++c) {
                    m_grid[r][c].isFrontier = false;
                }
            }
            ensureEndReachable();
            emit gridChanged();
            emit statusUpdated("Maze Generated (DFS)");
            return false;
        }

        // Clear previous frontier highlights
        for (int r = 0; r < m_height; ++r) {
            for (int c = 0; c < m_width; ++c) {
                m_grid[r][c].isFrontier = false;
            }
        }

        QPoint curr = m_genStack.back();
        m_grid[curr.y()][curr.x()].isFrontier = true; // Highlight current head

        std::vector<QPoint> neighbors = getNeighbors(curr, 2);
        std::vector<QPoint> unvisited;

        for (const auto& n : neighbors) {
            if (m_grid[n.y()][n.x()].isWall) {
                unvisited.push_back(n);
            }
        }

        if (!unvisited.empty()) {
            // Pick random neighbor
            std::uniform_int_distribution<size_t> dist(0, unvisited.size() - 1);
            QPoint next = unvisited[dist(m_rng)];

            // Carve the cell in between
            int midY = curr.y() + (next.y() - curr.y()) / 2;
            int midX = curr.x() + (next.x() - curr.x()) / 2;
            m_grid[midY][midX].isWall = false;
            m_grid[next.y()][next.x()].isWall = false;

            m_genStack.push_back(next);
            m_grid[next.y()][next.x()].isFrontier = true; // Highlight new head
        } else {
            m_genStack.pop_back();
            if (!m_genStack.empty()) {
                m_grid[m_genStack.back().y()][m_genStack.back().x()].isFrontier = true;
            }
        }
    }
    else if (m_activeGenType == GeneratorType::Prim) {
        if (m_primCandidates.empty()) {
            m_generating = false;
            m_finished = true;
            m_hasGeneratedMaze = true;
            // Clear frontier colors
            for (int r = 0; r < m_height; ++r) {
                for (int c = 0; c < m_width; ++c) {
                    m_grid[r][c].isFrontier = false;
                }
            }
            ensureEndReachable();
            emit gridChanged();
            emit statusUpdated("Maze Generated (Prim's)");
            return false;
        }

        // Pick a random candidate
        std::uniform_int_distribution<size_t> dist(0, m_primCandidates.size() - 1);
        size_t index = dist(m_rng);
        QPoint next = m_primCandidates[index];
        m_primCandidates.erase(m_primCandidates.begin() + index);
        m_grid[next.y()][next.x()].isFrontier = false;

        if (m_grid[next.y()][next.x()].isWall) {
            m_grid[next.y()][next.x()].isWall = false;

            // Connect to a random visited neighbor
            std::vector<QPoint> neighbors = getNeighbors(next, 2);
            std::vector<QPoint> visited;
            for (const auto& n : neighbors) {
                if (!m_grid[n.y()][n.x()].isWall) {
                    visited.push_back(n);
                }
            }

            if (!visited.empty()) {
                std::uniform_int_distribution<size_t> vDist(0, visited.size() - 1);
                QPoint prev = visited[vDist(m_rng)];
                int midY = next.y() + (prev.y() - next.y()) / 2;
                int midX = next.x() + (prev.x() - next.x()) / 2;
                m_grid[midY][midX].isWall = false;
            }

            // Add new unvisited neighbors to candidates
            std::vector<QPoint> nextNeighbors = getNeighbors(next, 2);
            for (const auto& n : nextNeighbors) {
                if (m_grid[n.y()][n.x()].isWall && !m_grid[n.y()][n.x()].isFrontier) {
                    m_primCandidates.push_back(n);
                    m_grid[n.y()][n.x()].isFrontier = true;
                }
            }
        }
    }
    else if (m_activeGenType == GeneratorType::Division) {
        if (m_divisionStack.empty()) {
            m_generating = false;
            m_finished = true;
            m_hasGeneratedMaze = true;
            ensureEndReachable();
            emit gridChanged();
            emit statusUpdated("Maze Generated (Recursive Division)");
            return false;
        }

        DivisionChamber chamber = m_divisionStack.back();
        m_divisionStack.pop_back();

        int w = chamber.xEnd - chamber.xStart;
        int h = chamber.yEnd - chamber.yStart;

        if (w < 2 || h < 2) {
            return true; // Too small, continue stepping
        }

        bool horizontal = false;
        if (w > h) {
            horizontal = false;
        } else if (h > w) {
            horizontal = true;
        } else {
            std::uniform_int_distribution<int> coin(0, 1);
            horizontal = (coin(m_rng) == 0);
        }

        if (horizontal) {
            // Choose even y coordinate for wall inside [chamber.yStart + 1, chamber.yEnd - 1]
            int yRange = (chamber.yEnd - 1) - (chamber.yStart + 1);
            if (yRange < 0) return true;
            std::uniform_int_distribution<int> yDist(0, yRange / 2);
            int wy = chamber.yStart + 1 + 2 * yDist(m_rng);

            // Choose odd x coordinate for passage inside [chamber.xStart, chamber.xEnd]
            int xRange = chamber.xEnd - chamber.xStart;
            std::uniform_int_distribution<int> xDist(0, xRange / 2);
            int px = chamber.xStart + 2 * xDist(m_rng);

            // Draw the wall
            for (int col = chamber.xStart - 1; col <= chamber.xEnd + 1; ++col) {
                if (col != px) {
                    m_grid[wy][col].isWall = true;
                }
            }

            // Push sub-chambers
            m_divisionStack.push_back(DivisionChamber{chamber.xStart, chamber.xEnd, chamber.yStart, wy - 1});
            m_divisionStack.push_back(DivisionChamber{chamber.xStart, chamber.xEnd, wy + 1, chamber.yEnd});
        } else {
            // Choose even x coordinate for wall inside [chamber.xStart + 1, chamber.xEnd - 1]
            int xRange = (chamber.xEnd - 1) - (chamber.xStart + 1);
            if (xRange < 0) return true;
            std::uniform_int_distribution<int> xDist(0, xRange / 2);
            int wx = chamber.xStart + 1 + 2 * xDist(m_rng);

            // Choose odd y coordinate for passage inside [chamber.yStart, chamber.yEnd]
            int yRange = chamber.yEnd - chamber.yStart;
            std::uniform_int_distribution<int> yDist(0, yRange / 2);
            int py = chamber.yStart + 2 * yDist(m_rng);

            // Draw the wall
            for (int r = chamber.yStart - 1; r <= chamber.yEnd + 1; ++r) {
                if (r != py) {
                    m_grid[r][wx].isWall = true;
                }
            }

            // Push sub-chambers
            m_divisionStack.push_back(DivisionChamber{chamber.xStart, wx - 1, chamber.yStart, chamber.yEnd});
            m_divisionStack.push_back(DivisionChamber{wx + 1, chamber.xEnd, chamber.yStart, chamber.yEnd});
        }
    }

    emit gridChanged();
    return true;
}

void MazeModel::generateInstant(GeneratorType type)
{
    initGeneration(type);
    while (stepGeneration()) {
        // Spin until completion
    }
}

void MazeModel::cancelGeneration()
{
    if (!m_generating) {
        return;
    }

    m_generating = false;
    m_finished = false;
    m_hasGeneratedMaze = false;

    m_genStack.clear();
    m_primCandidates.clear();
    m_divisionStack.clear();

    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            m_grid[r][c].isFrontier = false;
        }
    }

    emit gridChanged();
    emit statusUpdated("Generation cancelled.");
}

