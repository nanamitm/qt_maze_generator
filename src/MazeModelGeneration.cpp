#include "MazeModel.h"

#include "generators/GeneratorCatalog.h"

// ----------------- Generation -----------------

void MazeModel::initGeneration(const QString& generatorId)
{
    const GeneratorInfo *info = findGenerator(generatorId);
    if (!info) {
        emit statusUpdated(QString("Unknown generator: %1").arg(generatorId));
        return;
    }

    m_hasGeneratedMaze = false;
    m_generator = info->make(m_rng);
    m_generatorName = info->displayName;

    // The generator owns the starting grid state: some fill it with walls and
    // carve outwards, others start from an open room and add walls.
    m_generator->init(m_grid);
    ensureEndpointConnections();

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
