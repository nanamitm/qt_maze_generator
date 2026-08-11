#include "MazeModel.h"

#include "generators/GeneratorCatalog.h"

// ----------------- Generation -----------------

int MazeModel::generatorStepScale() const
{
    return m_generator ? m_generator->stepScale() : 1;
}

void MazeModel::initGeneration(const QString& generatorId)
{
    const GeneratorInfo *info = findGenerator(generatorId);
    if (!info) {
        emit statusUpdated(QString("Unknown generator: %1").arg(generatorId));
        return;
    }

    m_hasGeneratedMaze = false;
    m_rng.seed(m_seed);
    m_generator = info->make(m_rng);
    m_generatorName = info->displayName;

    // The generator owns the grid completely from here until it finishes: some
    // fill it with walls and carve outwards, others start from an open room and
    // add walls. Nothing may carve behind its back. Pre-carving the endpoints
    // here used to break the algorithms that read the grid as their own visited
    // set: Wilson would treat the pre-carved end cell as part of the maze and
    // close a loop through it, and Aldous-Broder would wait forever for a cell
    // that was already open to be discovered. The endpoints are joined by
    // ensureEndReachable() once generation is done.
    m_generator->init(m_grid);

    m_generating = true;
    m_solving = false;
    m_finished = false;

    emit gridChanged();
    emit statusUpdated("Generating...");
}

bool MazeModel::stepGeneration()
{
    if (!m_generating || !m_generator) return false;

    if (!m_generator->step(m_grid)) {
        m_generating = false;
        m_finished = true;
        m_hasGeneratedMaze = true;

        m_grid.clearFrontier();
        ensureEndReachable();

        emit gridChanged();
        emit statusUpdated(QString("Maze Generated: %1").arg(m_generatorName));
        return false;
    }

    emit gridChanged();
    return true;
}

void MazeModel::generateInstant(const QString& generatorId)
{
    initGeneration(generatorId);
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

    m_generator.reset();
    m_grid.clearFrontier();

    emit gridChanged();
    emit statusUpdated("Generation cancelled.");
}
