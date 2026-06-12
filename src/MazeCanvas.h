#ifndef MAZECANVAS_H
#define MAZECANVAS_H

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <vector>
#include "MazeModel.h"

class MazeCanvas : public QWidget {
    Q_OBJECT

public:
    explicit MazeCanvas(QWidget *parent = nullptr);

    void setModel(MazeModel *model);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    MazeModel *m_model = nullptr;

    enum class MouseMode {
        Idle,
        DrawingWall,
        ErasingWall,
        DraggingStart,
        DraggingEnd
    };

    MouseMode m_mouseMode = MouseMode::Idle;

    // Helper to convert widget coordinate to grid cell coordinate
    QPoint getCellFromPos(const QPoint& pos) const;
    QRectF cellRect(int row, int col) const;

    // Cell geometry cache. Even rows/columns are rendered as thin walls,
    // odd rows/columns as wider passages.
    std::vector<double> m_xEdges;
    std::vector<double> m_yEdges;
    double m_passageScale = 1.0;
    double m_wallScale = 0.32;
    void updateCellMetrics();
};

#endif // MAZECANVAS_H
