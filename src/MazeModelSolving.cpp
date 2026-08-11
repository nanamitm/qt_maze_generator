#include "MazeModel.h"

#include "solvers/SolverCatalog.h"

// ----------------- Solving -----------------

int MazeModel::solverStepScale() const
{
    return m_solver ? m_solver->stepScale() : 1;
}

void MazeModel::initSolving(const QString& solverId)
{
    if (!m_hasGeneratedMaze) {
        emit statusUpdated("Generate a maze before solving.");
        emit gridChanged();
        return;
    }

    const SolverInfo *info = findSolver(solverId);
    if (!info) {
        emit statusUpdated(QString("Unknown solver: %1").arg(solverId));
        return;
    }

    clearPathOnly();

    m_solver = info->make();
    m_solverName = info->displayName;

    m_solving = true;
    m_generating = false;
    m_finished = false;

    m_solver->init(m_grid, m_startPos, m_endPos);

    emit gridChanged();
    emit statusUpdated("Solving...");
}

bool MazeModel::stepSolving()
{
    if (!m_solving || !m_solver) return false;

    if (!m_solver->step(m_grid)) {
        m_solving = false;
        m_finished = true;

        const int pathLength = m_solver->pathLength();
        emit gridChanged();
        if (pathLength > 0) {
            emit statusUpdated(QString("Solved (%1)! Path length: %2")
                                   .arg(m_solverName).arg(pathLength));
        } else {
            emit statusUpdated("Solving completed: No Path Found.");
        }
        return false;
    }

    emit gridChanged();
    return true;
}

void MazeModel::solveInstant(const QString& solverId)
{
    initSolving(solverId);
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
    m_solver.reset();

    // Visited highlights are deliberately kept, so a cancelled solve still
    // shows how far it got.
    for (int r = 0; r < m_grid.height(); ++r) {
        for (int c = 0; c < m_grid.width(); ++c) {
            m_grid[r][c].isFrontier = false;
            m_grid[r][c].isSolution = false;
        }
    }

    emit gridChanged();
    emit statusUpdated("Solving cancelled.");
}
