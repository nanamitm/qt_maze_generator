#include "generators/GeneratorCatalog.h"

#include "generators/BinaryTreeGenerator.h"
#include "generators/DfsGenerator.h"
#include "generators/DivisionGenerator.h"
#include "generators/GrowingTreeGenerator.h"
#include "generators/HuntAndKillGenerator.h"
#include "generators/PrimGenerator.h"
#include "generators/SidewinderGenerator.h"

namespace {

template <typename T>
std::function<std::unique_ptr<MazeGenerator>(std::mt19937 &)> factory()
{
    return [](std::mt19937 &rng) -> std::unique_ptr<MazeGenerator> {
        return std::make_unique<T>(rng);
    };
}

std::function<std::unique_ptr<MazeGenerator>(std::mt19937 &)> growingTree(GrowingTreePolicy policy)
{
    return [policy](std::mt19937 &rng) -> std::unique_ptr<MazeGenerator> {
        return std::make_unique<GrowingTreeGenerator>(rng, policy);
    };
}

} // namespace

const std::vector<GeneratorInfo>& generatorCatalog()
{
    static const std::vector<GeneratorInfo> catalog = {
        {QStringLiteral("dfs"),      QStringLiteral("Recursive Backtracker (DFS)"), factory<DfsGenerator>()},
        {QStringLiteral("growingtree"), QStringLiteral("Growing Tree"),            growingTree(GrowingTreePolicy::Mixed)},
        {QStringLiteral("growingtree-oldest"), QStringLiteral("Growing Tree (oldest)"), growingTree(GrowingTreePolicy::Oldest)},
        {QStringLiteral("huntkill"), QStringLiteral("Hunt-and-Kill"),             factory<HuntAndKillGenerator>()},
        {QStringLiteral("prim"),     QStringLiteral("Prim's Algorithm"),            factory<PrimGenerator>()},
        {QStringLiteral("division"), QStringLiteral("Recursive Division"),          factory<DivisionGenerator>()},
        {QStringLiteral("binarytree"), QStringLiteral("Binary Tree"),               factory<BinaryTreeGenerator>()},
        {QStringLiteral("sidewinder"), QStringLiteral("Sidewinder"),                factory<SidewinderGenerator>()},
    };
    return catalog;
}

const GeneratorInfo* findGenerator(const QString& id)
{
    for (const GeneratorInfo& info : generatorCatalog()) {
        if (info.id == id) {
            return &info;
        }
    }
    return nullptr;
}

QString defaultGeneratorId()
{
    return generatorCatalog().front().id;
}
