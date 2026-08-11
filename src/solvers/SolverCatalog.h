#ifndef SOLVERCATALOG_H
#define SOLVERCATALOG_H

#include <QString>

#include <functional>
#include <memory>
#include <vector>

class MazeSolver;

// One entry per pathfinding algorithm, mirroring GeneratorInfo. Adding a solver
// means writing its class and appending a row here: the solver combo box and
// the self-test both read this list.
struct SolverInfo {
    QString id;          // stable identifier, safe to persist
    QString displayName; // shown in the UI
    std::function<std::unique_ptr<MazeSolver>()> make;
};

const std::vector<SolverInfo>& solverCatalog();

// Returns nullptr when the id is unknown.
const SolverInfo* findSolver(const QString& id);

QString defaultSolverId();

#endif // SOLVERCATALOG_H
