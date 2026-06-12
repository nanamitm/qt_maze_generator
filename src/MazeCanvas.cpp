#include "MazeCanvas.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <algorithm>
#include <cmath>

MazeCanvas::MazeCanvas(QWidget *parent)
    : QWidget(parent)
{
    // Enable mouse tracking to capture drag movements
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void MazeCanvas::setModel(MazeModel *model)
{
    m_model = model;
    connect(m_model, &MazeModel::gridChanged, this, qOverload<>(&QWidget::update));
    updateCellMetrics();
    update();
}

void MazeCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!m_model) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int rows = m_model->height();
    int cols = m_model->width();

    updateCellMetrics();

    // 1. Draw Background
    painter.fillRect(rect(), QColor("#0d0e12"));

    // 2. Draw Grid Cells
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const Cell& cell = m_model->cell(r, c);
            const QRectF rect = cellRect(r, c);

            if (cell.isWall) {
                // Sleek dark graphite wall
                painter.fillRect(rect, QColor("#1e2230"));
            } else {
                // Open path background
                painter.fillRect(rect, QColor("#0d0e12"));

                // Visited overlay (soft translucent purple/blue)
                if (cell.isVisited) {
                    painter.fillRect(rect, QColor(99, 102, 241, 100)); // Indigo translucent
                }

                // Frontier overlay (neon cyan)
                if (cell.isFrontier) {
                    painter.fillRect(rect, QColor(6, 182, 212, 180)); // Cyan translucent
                }

                // Solution path overlay (vibrant neon orange/gold)
                if (cell.isSolution) {
                    painter.fillRect(rect, QColor(249, 115, 22, 180)); // Orange translucent
                }
            }
        }
    }

    // 3. Draw Grid Lines (thin, semi-transparent)
    painter.setPen(QPen(QColor(38, 41, 54, 80), 1));
    for (double y : m_yEdges) {
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
    }
    for (double x : m_xEdges) {
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
    }

    // 4. Draw Start and End Markers (glowing neon circles)
    QPoint start = m_model->startPos();
    QPoint end = m_model->endPos();

    // Start Node (Neon Green)
    QRectF startCell = cellRect(start.y(), start.x());
    const double startInset = std::min(startCell.width(), startCell.height()) * 0.12;
    QRectF startRect = startCell.adjusted(startInset, startInset, -startInset, -startInset);
    QRadialGradient startGrad(startRect.center(), std::min(startRect.width(), startRect.height()) / 2);
    startGrad.setColorAt(0, QColor("#10b981"));
    startGrad.setColorAt(0.7, QColor("#059669"));
    startGrad.setColorAt(1, QColor(5, 150, 105, 100));
    painter.setBrush(startGrad);
    painter.setPen(QPen(QColor("#047857"), 1.5));
    painter.drawEllipse(startRect);

    // Start text "S"
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(static_cast<int>(std::max(8.0, std::min(startRect.width(), startRect.height()) * 0.5)));
    painter.setFont(font);
    painter.drawText(startRect, Qt::AlignCenter, "S");

    // End Node (Neon Crimson)
    QRectF endCell = cellRect(end.y(), end.x());
    const double endInset = std::min(endCell.width(), endCell.height()) * 0.12;
    QRectF endRect = endCell.adjusted(endInset, endInset, -endInset, -endInset);
    QRadialGradient endGrad(endRect.center(), std::min(endRect.width(), endRect.height()) / 2);
    endGrad.setColorAt(0, QColor("#ef4444"));
    endGrad.setColorAt(0.7, QColor("#dc2626"));
    endGrad.setColorAt(1, QColor(220, 38, 38, 100));
    painter.setBrush(endGrad);
    painter.setPen(QPen(QColor("#b91c1c"), 1.5));
    painter.drawEllipse(endRect);

    // End text "E"
    painter.setPen(Qt::white);
    painter.drawText(endRect, Qt::AlignCenter, "E");
}

QPoint MazeCanvas::getCellFromPos(const QPoint& pos) const
{
    if (!m_model) return QPoint(-1, -1);
    if (m_xEdges.size() < 2 || m_yEdges.size() < 2) return QPoint(-1, -1);

    auto xIt = std::upper_bound(m_xEdges.begin(), m_xEdges.end(), pos.x());
    auto yIt = std::upper_bound(m_yEdges.begin(), m_yEdges.end(), pos.y());
    int col = static_cast<int>(std::distance(m_xEdges.begin(), xIt)) - 1;
    int row = static_cast<int>(std::distance(m_yEdges.begin(), yIt)) - 1;

    col = std::clamp(col, 0, m_model->width() - 1);
    row = std::clamp(row, 0, m_model->height() - 1);

    return QPoint(col, row);
}

QRectF MazeCanvas::cellRect(int row, int col) const
{
    if (row < 0 || col < 0 || row + 1 >= static_cast<int>(m_yEdges.size())
        || col + 1 >= static_cast<int>(m_xEdges.size())) {
        return QRectF();
    }

    return QRectF(
        QPointF(m_xEdges[col], m_yEdges[row]),
        QPointF(m_xEdges[col + 1], m_yEdges[row + 1])
    );
}

void MazeCanvas::updateCellMetrics()
{
    if (!m_model) return;

    const int cols = m_model->width();
    const int rows = m_model->height();
    m_xEdges.assign(cols + 1, 0.0);
    m_yEdges.assign(rows + 1, 0.0);

    auto weightForIndex = [this](int index) {
        return (index % 2 == 0) ? m_wallScale : m_passageScale;
    };

    double totalXWeight = 0.0;
    for (int c = 0; c < cols; ++c) {
        totalXWeight += weightForIndex(c);
    }
    double totalYWeight = 0.0;
    for (int r = 0; r < rows; ++r) {
        totalYWeight += weightForIndex(r);
    }

    const double unitW = static_cast<double>(std::max(1, width())) / totalXWeight;
    const double unitH = static_cast<double>(std::max(1, height())) / totalYWeight;

    for (int c = 0; c < cols; ++c) {
        m_xEdges[c + 1] = m_xEdges[c] + weightForIndex(c) * unitW;
    }
    for (int r = 0; r < rows; ++r) {
        m_yEdges[r + 1] = m_yEdges[r] + weightForIndex(r) * unitH;
    }

    m_xEdges.back() = width();
    m_yEdges.back() = height();
}

void MazeCanvas::mousePressEvent(QMouseEvent *event)
{
    if (!m_model) return;
    if (m_model->isGenerating() || m_model->isSolving()) return;

    QPoint cellPos = getCellFromPos(event->pos());
    int r = cellPos.y();
    int c = cellPos.x();

    if (!m_model->isValid(r, c)) return;

    // Check if dragging start/end
    if (cellPos == m_model->startPos()) {
        m_mouseMode = MouseMode::DraggingStart;
    } else if (cellPos == m_model->endPos()) {
        m_mouseMode = MouseMode::DraggingEnd;
    } else {
        // Only allow wall drawing if idle
        const Cell& cell = m_model->cell(r, c);
        if (cell.isWall) {
            m_mouseMode = MouseMode::ErasingWall;
            m_model->setWall(r, c, false);
        } else {
            m_mouseMode = MouseMode::DrawingWall;
            m_model->setWall(r, c, true);
        }
    }
}

void MazeCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_model) return;
    if (m_model->isGenerating() || m_model->isSolving()) return;

    QPoint cellPos = getCellFromPos(event->pos());
    int r = cellPos.y();
    int c = cellPos.x();

    if (!m_model->isValid(r, c)) return;

    if (m_mouseMode == MouseMode::DraggingStart) {
        m_model->setStartPos(cellPos);
    }
    else if (m_mouseMode == MouseMode::DraggingEnd) {
        m_model->setEndPos(cellPos);
    }
    else if (!m_model->isGenerating() && !m_model->isSolving()) {
        if (m_mouseMode == MouseMode::DrawingWall) {
            m_model->setWall(r, c, true);
        } else if (m_mouseMode == MouseMode::ErasingWall) {
            m_model->setWall(r, c, false);
        }
    }
}

void MazeCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_mouseMode = MouseMode::Idle;
}

void MazeCanvas::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateCellMetrics();
}
