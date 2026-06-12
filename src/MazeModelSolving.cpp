#include "MazeModel.h"


// ----------------- Solving -----------------

void MazeModel::initSolving(SolverType type)
{
    if (!m_hasGeneratedMaze) {
        emit statusUpdated("Generate a maze before solving.");
        emit gridChanged();
        return;
    }

    m_activeSolveType = type;

    clearPathOnly();

    m_solving = true;
    m_generating = false;
    m_finished = false;

    m_parentMap.assign(m_height, std::vector<QPoint>(m_width, QPoint(-1, -1)));
    m_closedSet.assign(m_height, std::vector<bool>(m_width, false));

    m_grid[m_startPos.y()][m_startPos.x()].isFrontier = true;

    if (type == SolverType::BFS) {
        m_solveQueue.clear();
        m_solveQueue.push_back(m_startPos);
    }
    else if (type == SolverType::DFS) {
        m_solveStack.clear();
        m_solveStack.push_back(m_startPos);
    }
    else if (type == SolverType::AStar) {
        while (!m_solveMinHeap.empty()) m_solveMinHeap.pop();
        m_gScore.assign(m_height, std::vector<double>(m_width, 1e9));
        m_gScore[m_startPos.y()][m_startPos.x()] = 0.0;
        m_solveMinHeap.push(AStarNode{m_startPos, heuristic(m_startPos, m_endPos)});
    }

    emit gridChanged();
    emit statusUpdated("Solving...");
}

bool MazeModel::stepSolving()
{
    if (!m_solving) return false;

    if (m_activeSolveType == SolverType::BFS) {
        if (m_solveQueue.empty()) {
            m_solving = false;
            m_finished = true;
            emit gridChanged();
            emit statusUpdated("Solving completed: No Path Found.");
            return false;
        }

        QPoint curr = m_solveQueue.front();
        m_solveQueue.pop_front();

        m_grid[curr.y()][curr.x()].isFrontier = false;
        m_grid[curr.y()][curr.x()].isVisited = true;

        if (curr == m_endPos) {
            // Reconstruct path
            int pathLen = 0;
            QPoint p = m_endPos;
            while (p != QPoint(-1, -1)) {
                m_grid[p.y()][p.x()].isSolution = true;
                p = m_parentMap[p.y()][p.x()];
                pathLen++;
            }
            m_solving = false;
            m_finished = true;
            emit gridChanged();
            emit statusUpdated(QString("Solved (BFS)! Path length: %1").arg(pathLen));
            return false;
        }

        std::vector<QPoint> neighbors = getWalkableNeighbors(curr);
        for (const auto& next : neighbors) {
            if (!m_grid[next.y()][next.x()].isVisited && !m_grid[next.y()][next.x()].isFrontier) {
                m_parentMap[next.y()][next.x()] = curr;
                m_grid[next.y()][next.x()].isFrontier = true;
                m_solveQueue.push_back(next);
            }
        }
    }
    else if (m_activeSolveType == SolverType::DFS) {
        if (m_solveStack.empty()) {
            m_solving = false;
            m_finished = true;
            emit gridChanged();
            emit statusUpdated("Solving completed: No Path Found.");
            return false;
        }

        QPoint curr = m_solveStack.back();
        m_solveStack.pop_back();

        m_grid[curr.y()][curr.x()].isFrontier = false;
        m_grid[curr.y()][curr.x()].isVisited = true;

        if (curr == m_endPos) {
            // Reconstruct path
            int pathLen = 0;
            QPoint p = m_endPos;
            while (p != QPoint(-1, -1)) {
                m_grid[p.y()][p.x()].isSolution = true;
                p = m_parentMap[p.y()][p.x()];
                pathLen++;
            }
            m_solving = false;
            m_finished = true;
            emit gridChanged();
            emit statusUpdated(QString("Solved (DFS)! Path length: %1").arg(pathLen));
            return false;
        }

        std::vector<QPoint> neighbors = getWalkableNeighbors(curr);
        for (const auto& next : neighbors) {
            if (!m_grid[next.y()][next.x()].isVisited && !m_grid[next.y()][next.x()].isFrontier) {
                m_parentMap[next.y()][next.x()] = curr;
                m_grid[next.y()][next.x()].isFrontier = true;
                m_solveStack.push_back(next);
            }
        }
    }
    else if (m_activeSolveType == SolverType::AStar) {
        if (m_solveMinHeap.empty()) {
            m_solving = false;
            m_finished = true;
            emit gridChanged();
            emit statusUpdated("Solving completed: No Path Found.");
            return false;
        }

        AStarNode node = m_solveMinHeap.top();
        m_solveMinHeap.pop();
        QPoint curr = node.pos;

        if (m_closedSet[curr.y()][curr.x()]) {
            return true; // Already processed
        }
        m_closedSet[curr.y()][curr.x()] = true;

        m_grid[curr.y()][curr.x()].isFrontier = false;
        m_grid[curr.y()][curr.x()].isVisited = true;

        if (curr == m_endPos) {
            // Reconstruct path
            int pathLen = 0;
            QPoint p = m_endPos;
            while (p != QPoint(-1, -1)) {
                m_grid[p.y()][p.x()].isSolution = true;
                p = m_parentMap[p.y()][p.x()];
                pathLen++;
            }
            m_solving = false;
            m_finished = true;
            emit gridChanged();
            emit statusUpdated(QString("Solved (A*)! Path length: %1").arg(pathLen));
            return false;
        }

        std::vector<QPoint> neighbors = getWalkableNeighbors(curr);
        for (const auto& next : neighbors) {
            if (m_closedSet[next.y()][next.x()]) continue;

            double tentative_g = m_gScore[curr.y()][curr.x()] + 1.0;
            if (tentative_g < m_gScore[next.y()][next.x()]) {
                m_parentMap[next.y()][next.x()] = curr;
                m_gScore[next.y()][next.x()] = tentative_g;
                double fScore = tentative_g + heuristic(next, m_endPos);
                m_grid[next.y()][next.x()].isFrontier = true;
                m_solveMinHeap.push(AStarNode{next, fScore});
            }
        }
    }

    emit gridChanged();
    return true;
}

void MazeModel::solveInstant(SolverType type)
{
    initSolving(type);
    while (stepSolving()) {
        // Spin until completion
    }
}

void MazeModel::cancelSolving()
{
    if (!m_solving) {
        return;
    }

    m_solving = false;
    m_finished = false;

    m_solveQueue.clear();
    m_solveStack.clear();
    while (!m_solveMinHeap.empty()) {
        m_solveMinHeap.pop();
    }
    m_parentMap.clear();
    m_gScore.clear();
    m_closedSet.clear();

    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            m_grid[r][c].isFrontier = false;
            m_grid[r][c].isSolution = false;
        }
    }

    emit gridChanged();
    emit statusUpdated("Solving cancelled.");
}
