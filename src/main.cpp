#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

#include <array>

#include "MainWindow.h"
#include "MazeModel.h"

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

QString generatorName(GeneratorType type)
{
    switch (type) {
    case GeneratorType::DFS:
        return "DFS";
    case GeneratorType::Prim:
        return "Prim";
    case GeneratorType::Division:
        return "Division";
    }
    return "Unknown";
}

QString solverName(SolverType type)
{
    switch (type) {
    case SolverType::BFS:
        return "BFS";
    case SolverType::DFS:
        return "DFS";
    case SolverType::AStar:
        return "AStar";
    }
    return "Unknown";
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

int runSelfTest()
{
    const std::array<GeneratorType, 3> generators = {
        GeneratorType::DFS,
        GeneratorType::Prim,
        GeneratorType::Division,
    };
    const std::array<SolverType, 3> solvers = {
        SolverType::BFS,
        SolverType::DFS,
        SolverType::AStar,
    };

    for (GeneratorType generator : generators) {
        for (SolverType solver : solvers) {
            MazeModel model;
            model.setSize(31, 31);
            model.generateInstant(generator);

            if (!model.hasGeneratedMaze()) {
                qCritical() << "Generation did not finish:" << generatorName(generator);
                return 1;
            }

            model.solveInstant(solver);
            const int pathLength = solutionCellCount(model);
            if (pathLength <= 0) {
                qCritical() << "Solver did not find a path:"
                            << generatorName(generator) << solverName(solver);
                return 1;
            }
        }
    }

    qInfo() << "Self-test passed.";
    return 0;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Qt6 C++ Maze Generator");
    a.setApplicationVersion("1.0.0");
    a.setWindowIcon(createAppIcon());

    if (QCoreApplication::arguments().contains("--self-test")) {
        return runSelfTest();
    }

    MainWindow w;
    w.setWindowIcon(a.windowIcon());
    w.show();

    return a.exec();
}
