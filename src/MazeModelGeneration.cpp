#include "MazeModel.h"

#include "generators/DfsGenerator.h"
#include "generators/DivisionGenerator.h"
#include "generators/PrimGenerator.h"

namespace {

struct GeneratorEntry {
    std::unique_ptr<MazeGenerator> generator;
    QString name;
};

GeneratorEntry makeGenerator(GeneratorType type, std::mt19937 &rng)
{
    switch (type) {
    case GeneratorType::DFS:
        return {std::make_unique<DfsGenerator>(rng), QStringLiteral("DFS")};
    case GeneratorType::Prim:
        return {std::make_unique<PrimGenerator>(rng), QStringLiteral("Prim's")};
    case GeneratorType::Division:
        return {std::make_unique<DivisionGenerator>(rng), QStringLiteral("Recursive Division")};
    }
    return {std::make_unique<DfsGenerator>(rng), QStringLiteral("DFS")};
}

} // namespace

// ----------------- Generation -----------------

void MazeModel::initGeneration(GeneratorType type)
{
    m_hasGeneratedMaze = false;

    GeneratorEntry entry = makeGenerator(type, m_rng);
    m_generator = std::move(entry.generator);
    m_generatorName = entry.name;

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
        emit statusUpdated(QString("Maze Generated (%1)").arg(m_generatorName));
        return false;
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

    m_generator.reset();
    m_grid.clearFrontier();

    emit gridChanged();
    emit statusUpdated("Generation cancelled.");
}
