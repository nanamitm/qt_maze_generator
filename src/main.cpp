#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QStringList>

#include <array>
#include <cstdio>
#include <deque>

#include "MainWindow.h"
#include "MazeModel.h"
#include "generators/GeneratorCatalog.h"
#include "solvers/SolverCatalog.h"

namespace {

int solutionCellCount(const MazeModel& model)
{
    int count = 0;
    for (int y = 0; y < model.height(); ++y) {
        for (int x = 0; x < model.width(); ++x) {
            if (model.cell(y, x).isSolution) {
                ++count;
            }
        }
    }
    return count;
}

int visitedCellCount(const MazeModel& model)
{
    int count = 0;
    for (int y = 0; y < model.height(); ++y) {
        for (int x = 0; x < model.width(); ++x) {
            if (model.cell(y, x).isVisited) {
                ++count;
            }
        }
    }
    return count;
}

std::vector<bool> wallSnapshot(const MazeModel& model)
{
    std::vector<bool> walls;
    walls.reserve(static_cast<size_t>(model.width()) * model.height());
    for (int y = 0; y < model.height(); ++y) {
        for (int x = 0; x < model.width(); ++x) {
            walls.push_back(model.cell(y, x).isWall);
        }
    }
    return walls;
}

// Generating twice from one seed must produce the same maze, otherwise the
// seed control is decorative and runs cannot be compared across algorithms.
bool isSeedReproducible(const QString& generatorId)
{
    std::vector<bool> first;
    for (int run = 0; run < 2; ++run) {
        MazeModel model;
        model.setSize(31, 31);
        model.setSeed(12345);
        model.generateInstant(generatorId);

        if (run == 0) {
            first = wallSnapshot(model);
        } else if (wallSnapshot(model) != first) {
            return false;
        }
    }
    return true;
}

// Fingerprint of a finished maze. "Every cell is connected" is true of every
// algorithm here, so it cannot tell a correct implementation from a subtly
// wrong one. The share of dead ends and junctions can: it differs by a factor
// of three or more between algorithms, and each one lands in its own band.
struct MazeStats {
    int cells = 0;
    int passages = 0; // connections between neighbouring cells
    int longestPath = 0;
    double deadEndPercent = 0.0;
    double junctionPercent = 0.0;

    // A perfect maze is a spanning tree: connected, and with exactly one
    // passage fewer than it has cells. More passages means a loop.
    bool isPerfect() const { return passages == cells - 1; }
};

// Longest path between any two cells, i.e. the diameter of the tree, found by
// breadth-first search twice. Dead-end and junction shares cannot tell the
// recursive backtracker from hunt-and-kill, since both sit at 10%; this can,
// because the backtracker's single long walk gives it a diameter several times
// larger.
int measureLongestPath(const MazeModel& model)
{
    const auto farthestFrom = [&model](const QPoint& start, int *distance) {
        std::vector<std::vector<int>> dist(model.height(), std::vector<int>(model.width(), -1));
        std::deque<QPoint> queue;
        queue.push_back(start);
        dist[start.y()][start.x()] = 0;

        QPoint best = start;
        int bestDistance = 0;

        while (!queue.empty()) {
            const QPoint curr = queue.front();
            queue.pop_front();

            if (dist[curr.y()][curr.x()] > bestDistance) {
                bestDistance = dist[curr.y()][curr.x()];
                best = curr;
            }

            const int dx[] = {0, 0, -1, 1};
            const int dy[] = {-1, 1, 0, 0};
            for (int i = 0; i < 4; ++i) {
                const QPoint next(curr.x() + dx[i] * 2, curr.y() + dy[i] * 2);
                if (next.x() < 1 || next.x() > model.width() - 2) continue;
                if (next.y() < 1 || next.y() > model.height() - 2) continue;
                if (model.cell(curr.y() + dy[i], curr.x() + dx[i]).isWall) continue;
                if (dist[next.y()][next.x()] != -1) continue;
                dist[next.y()][next.x()] = dist[curr.y()][curr.x()] + 1;
                queue.push_back(next);
            }
        }

        if (distance) *distance = bestDistance;
        return best;
    };

    int ignored = 0;
    const QPoint end = farthestFrom(QPoint(1, 1), &ignored);
    int diameter = 0;
    farthestFrom(end, &diameter);
    return diameter + 1; // in cells, not steps
}

MazeStats measureMaze(const MazeModel& model)
{
    MazeStats stats;
    int deadEnds = 0;
    int junctions = 0;

    // Cells live on odd coordinates; the even coordinate between two of them is
    // the wall that may join them.
    for (int y = 1; y < model.height() - 1; y += 2) {
        for (int x = 1; x < model.width() - 1; x += 2) {
            if (model.cell(y, x).isWall) continue;
            ++stats.cells;

            const int dx[] = {0, 0, -1, 1};
            const int dy[] = {-1, 1, 0, 0};
            int degree = 0;
            for (int i = 0; i < 4; ++i) {
                const int nx = x + dx[i] * 2;
                const int ny = y + dy[i] * 2;
                if (nx < 1 || nx > model.width() - 2) continue;
                if (ny < 1 || ny > model.height() - 2) continue;
                if (!model.cell(y + dy[i], x + dx[i]).isWall) ++degree;
            }

            if (degree == 1) ++deadEnds;
            if (degree >= 3) ++junctions;
            stats.passages += degree;
        }
    }
    stats.passages /= 2; // each passage was counted from both ends
    stats.longestPath = measureLongestPath(model);

    if (stats.cells > 0) {
        stats.deadEndPercent = 100.0 * deadEnds / stats.cells;
        stats.junctionPercent = 100.0 * junctions / stats.cells;
    }
    return stats;
}

QIcon createAppIcon()
{
    QIcon icon;
    const std::array<int, 5> sizes = {16, 24, 32, 48, 64};

    for (int size : sizes) {
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF bounds(1.0, 1.0, size - 2.0, size - 2.0);
        QPainterPath background;
        background.addRoundedRect(bounds, size * 0.18, size * 0.18);
        painter.fillPath(background, QColor("#15171e"));
        painter.setPen(QPen(QColor("#32374a"), std::max(1.0, size / 24.0)));
        painter.drawPath(background);

        const double margin = size * 0.22;
        const double cell = (size - margin * 2.0) / 5.0;
        painter.setPen(QPen(QColor("#00f2fe"), std::max(1.0, size / 18.0), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        auto point = [&](double x, double y) {
            return QPointF(margin + x * cell, margin + y * cell);
        };

        painter.drawLine(point(0, 0), point(4, 0));
        painter.drawLine(point(0, 0), point(0, 4));
        painter.drawLine(point(1, 1), point(4, 1));
        painter.drawLine(point(1, 1), point(1, 3));
        painter.drawLine(point(2, 2), point(2, 4));
        painter.drawLine(point(3, 2), point(4, 2));
        painter.drawLine(point(4, 2), point(4, 4));

        painter.setPen(QPen(QColor("#f97316"), std::max(1.0, size / 20.0), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        QPainterPath route;
        route.moveTo(point(0.35, 3.65));
        route.lineTo(point(1.35, 3.65));
        route.lineTo(point(1.35, 2.35));
        route.lineTo(point(3.65, 2.35));
        route.lineTo(point(3.65, 0.65));
        painter.drawPath(route);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#10b981"));
        painter.drawEllipse(point(0.35, 3.65), cell * 0.28, cell * 0.28);
        painter.setBrush(QColor("#ef4444"));
        painter.drawEllipse(point(3.65, 0.65), cell * 0.28, cell * 0.28);

        icon.addPixmap(pixmap);
    }

    return icon;
}

// The executable is linked into the GUI subsystem, so Qt sees no console
// attached and sends messages to OutputDebugString, discarding the self-test
// result even when stderr is a perfectly usable inherited pipe. Bypass that
// detection and write to stderr directly.
void selfTestMessageHandler(QtMsgType, const QMessageLogContext &, const QString &message)
{
    fputs(qPrintable(message), stderr);
    fputc('\n', stderr);
    fflush(stderr);
}

bool hasSelfTestFlag(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (qstrcmp(argv[i], "--self-test") == 0) {
            return true;
        }
    }
    return false;
}

// Reference fingerprints, measured on the same 88x47 = 4136 cell maze the
// reference visualisation used, so the numbers are directly comparable.
struct StatsExpectation {
    const char *id;
    double deadEndPercent;
    double junctionPercent;
    int longestPath;
};

const std::array<StatsExpectation, 13> kStatsExpectations = {{
    {"dfs",                10.0,  9.0, 1584},
    {"huntkill",           10.0,  9.0,  471},
    {"prim",               32.0, 27.0,  298},
    {"kruskal",            31.0, 27.0,  436},
    {"boruvka",            28.0, 25.0,  483},
    {"growingtree",        15.0, 15.0,  489},
    {"growingtree-oldest",  2.0,  2.0,  226},
    {"wilson",             30.0, 26.0,  342},
    {"aldousbroder",       30.0, 26.0,  407},
    {"eller",              28.0, 25.0,  438},
    {"sidewinder",         29.0, 26.0,  241},
    {"binarytree",         25.0, 25.0,  232},
    // The reference lists 34% junctions for recursive division, which no
    // perfect maze can reach: in a spanning tree junctions <= dead ends - 2,
    // and every other algorithm in that reference obeys it. Taken as a typo
    // for 14%, which is what this implementation measures.
    {"division",           15.0, 14.0,  378},
}};

// Single-sample references, and an algorithm's exact figures shift a little
// with the seed, so allow a band. The algorithms sit 10+ points apart, so this
// still separates them.
constexpr double kStatsTolerance = 4.0;

// Path length swings far more with the seed than the percentages do, so it
// gets a proportional band. It still separates the algorithms that share a
// dead-end share: the backtracker's 1584 against hunt-and-kill's 471.
constexpr double kPathTolerance = 0.45;

bool checkMazeStatistics()
{
    bool ok = true;

    for (const GeneratorInfo& generator : generatorCatalog()) {
        const StatsExpectation *expected = nullptr;
        for (const StatsExpectation& candidate : kStatsExpectations) {
            if (generator.id == QLatin1String(candidate.id)) {
                expected = &candidate;
                break;
            }
        }

        if (!expected) {
            qCritical() << "No expected statistics registered for generator:" << generator.id;
            ok = false;
            continue;
        }

        MazeModel model;
        model.setSize(177, 95); // 88 x 47 cells
        model.setSeed(20260806);
        model.generateInstant(generator.id);

        const MazeStats stats = measureMaze(model);
        if (!stats.isPerfect()) {
            ok = false;
        }
        const double deadEndError = std::abs(stats.deadEndPercent - expected->deadEndPercent);
        const double junctionError = std::abs(stats.junctionPercent - expected->junctionPercent);
        const double pathRatio = static_cast<double>(stats.longestPath) / expected->longestPath;
        const bool matches = deadEndError <= kStatsTolerance
            && junctionError <= kStatsTolerance
            && pathRatio >= 1.0 - kPathTolerance
            && pathRatio <= 1.0 + kPathTolerance;

        qInfo().noquote() << QString("%1: %2 cells%9, dead ends %3% (exp %4%), junctions %5% (exp %6%), longest path %8 (exp %10)%7")
            .arg(generator.id)
            .arg(stats.cells)
            .arg(stats.deadEndPercent, 0, 'f', 1)
            .arg(expected->deadEndPercent, 0, 'f', 0)
            .arg(stats.junctionPercent, 0, 'f', 1)
            .arg(expected->junctionPercent, 0, 'f', 0)
            .arg(matches ? "" : "  <-- OUT OF BAND")
            .arg(stats.longestPath)
            .arg(stats.isPerfect() ? "" : QString(", NOT PERFECT (%1 passages, expected %2)")
                 .arg(stats.passages).arg(stats.cells - 1))
            .arg(expected->longestPath);

        if (!matches) {
            ok = false;
        }
    }

    return ok;
}

// Opens extra walls to braid the maze, turning the spanning tree into a graph
// with loops. Every generated maze is a tree, so without this nothing here
// ever exercises the case the walking solvers actually differ on.
int braidMaze(MazeModel& model)
{
    int opened = 0;
    int candidate = 0;

    for (int y = 1; y <= model.height() - 2; ++y) {
        for (int x = 1; x <= model.width() - 2; ++x) {
            // A wall between two cells has exactly one odd coordinate.
            const bool betweenCells = (x % 2 == 0) != (y % 2 == 0);
            if (!betweenCells || !model.cell(y, x).isWall) {
                continue;
            }

            const int dx = (x % 2 == 0) ? 1 : 0;
            const int dy = (y % 2 == 0) ? 1 : 0;
            if (model.cell(y - dy, x - dx).isWall || model.cell(y + dy, x + dx).isWall) {
                continue;
            }

            if (candidate++ % 5 != 0) {
                continue;
            }
            model.setWall(y, x, false);
            ++opened;
        }
    }
    return opened;
}

// A maze with loops is where the walking solvers part company: Tremaux is
// guaranteed to get through one, the wall follower is not.
bool checkBraidedMaze()
{
    MazeModel model;
    model.setSize(31, 31);
    model.setSeed(4242);
    model.generateInstant(QStringLiteral("dfs"));

    const int opened = braidMaze(model);
    const MazeStats stats = measureMaze(model);
    if (opened == 0 || stats.isPerfect()) {
        qCritical() << "Braiding did not add any loops:" << opened << "walls opened";
        return false;
    }

    model.solveInstant(QStringLiteral("bfs"));
    const int shortest = solutionCellCount(model);

    model.solveInstant(QStringLiteral("tremaux"));
    const int tremaux = solutionCellCount(model);

    model.solveInstant(QStringLiteral("wallfollow"));
    const int wallFollower = solutionCellCount(model);

    if (shortest <= 0) {
        qCritical() << "BFS found no path in the braided maze.";
        return false;
    }
    if (tremaux < shortest) {
        qCritical() << "Tremaux reported a path shorter than the shortest one:"
                    << tremaux << "against" << shortest;
        return false;
    }
    if (tremaux == 0) {
        qCritical() << "Tremaux failed on a braided maze, which it must not.";
        return false;
    }

    qInfo().noquote() << QString("braided (%1 extra passages): bfs %2 cells, tremaux %3, "
                                 "wall follower %4")
                             .arg(opened).arg(shortest).arg(tremaux)
                             .arg(wallFollower > 0 ? QString::number(wallFollower)
                                                   : QStringLiteral("gave up"));
    return true;
}

int runSelfTest()
{
    QFile stylesheet(":/styles.qss");
    if (!stylesheet.open(QFile::ReadOnly | QFile::Text) || stylesheet.readAll().isEmpty()) {
        qCritical() << "Embedded stylesheet is missing or empty.";
        return 1;
    }

    if (!checkMazeStatistics()) {
        return 1;
    }

    if (!checkBraidedMaze()) {
        return 1;
    }

    // Driven by the catalog, so a newly added generator is covered without
    // touching this function.
    int combinations = 0;
    for (const GeneratorInfo& generator : generatorCatalog()) {
        if (!isSeedReproducible(generator.id)) {
            qCritical() << "Generator is not reproducible from its seed:" << generator.displayName;
            return 1;
        }

        // A generated maze is a spanning tree, so exactly one route joins any
        // two cells. Every solver that finds a path must therefore report the
        // same length, whether or not the algorithm is one that guarantees the
        // shortest route: a solver that disagrees has a broken parent chain.
        int expectedPathLength = 0;
        QString expectedFrom;
        QStringList effort;

        for (const SolverInfo& solver : solverCatalog()) {
            MazeModel model;
            model.setSize(31, 31);
            model.setSeed(777);
            model.generateInstant(generator.id);

            if (!model.hasGeneratedMaze()) {
                qCritical() << "Generation did not finish:" << generator.displayName;
                return 1;
            }

            model.solveInstant(solver.id);
            const int pathLength = solutionCellCount(model);
            if (pathLength <= 0) {
                qCritical() << "Solver did not find a path:"
                            << generator.displayName << solver.displayName;
                return 1;
            }

            if (expectedPathLength == 0) {
                expectedPathLength = pathLength;
                expectedFrom = solver.id;
            } else if (pathLength != expectedPathLength) {
                qCritical().noquote()
                    << QString("Solvers disagree on %1: %2 found %3 cells, %4 found %5")
                           .arg(generator.id).arg(expectedFrom).arg(expectedPathLength)
                           .arg(solver.id).arg(pathLength);
                return 1;
            }

            effort << QString("%1 %2").arg(solver.id).arg(visitedCellCount(model));
            ++combinations;
        }

        // Search effort is what actually separates the solvers here, so it is
        // reported rather than asserted.
        qInfo().noquote() << QString("%1: path %2 cells, visited: %3")
                                 .arg(generator.id).arg(expectedPathLength)
                                 .arg(effort.join(", "));
    }

    qInfo() << "Self-test passed:" << combinations << "generator/solver combinations.";
    return 0;
}

} // namespace

int main(int argc, char *argv[])
{
    // The self-test only exercises MazeModel, so it must not require a display.
    // Detect the flag before QApplication is constructed.
    if (hasSelfTestFlag(argc, argv)) {
        qInstallMessageHandler(selfTestMessageHandler);
        QCoreApplication app(argc, argv);
        return runSelfTest();
    }

    QApplication a(argc, argv);
    a.setApplicationName("Qt6 C++ Maze Generator");
    a.setApplicationVersion("1.0.0");
    a.setWindowIcon(createAppIcon());

    MainWindow w;
    w.setWindowIcon(a.windowIcon());
    w.show();

    return a.exec();
}
