#include "solvers/SolverCatalog.h"

#include "solvers/AStarSolver.h"
#include "solvers/BfsSolver.h"
#include "solvers/BidirectionalBfsSolver.h"
#include "solvers/DeadEndFillingSolver.h"
#include "solvers/DfsSolver.h"
#include "solvers/GreedySolver.h"

namespace {

template <typename T>
std::function<std::unique_ptr<MazeSolver>()> factory()
{
    return []() -> std::unique_ptr<MazeSolver> {
        return std::make_unique<T>();
    };
}

} // namespace

const std::vector<SolverInfo>& solverCatalog()
{
    static const std::vector<SolverInfo> catalog = {
        {QStringLiteral("bfs"),   QStringLiteral("Breadth-First Search (BFS)"), factory<BfsSolver>()},
        {QStringLiteral("bidi"),   QStringLiteral("Bidirectional BFS"),          factory<BidirectionalBfsSolver>()},
        {QStringLiteral("dfs"),   QStringLiteral("Depth-First Search (DFS)"),   factory<DfsSolver>()},
        {QStringLiteral("astar"),  QStringLiteral("A* Search Algorithm"),        factory<AStarSolver>()},
        {QStringLiteral("greedy"), QStringLiteral("Greedy Best-First Search"),  factory<GreedySolver>()},
        {QStringLiteral("deadend"), QStringLiteral("Dead-End Filling"),          factory<DeadEndFillingSolver>()},
    };
    return catalog;
}

const SolverInfo* findSolver(const QString& id)
{
    for (const SolverInfo& info : solverCatalog()) {
        if (info.id == id) {
            return &info;
        }
    }
    return nullptr;
}

QString defaultSolverId()
{
    return solverCatalog().front().id;
}
