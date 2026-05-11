#include "GraphWidget.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

#include "parser/Evaluator.h"

namespace {
constexpr int kMarginLeft   = 50;
constexpr int kMarginRight  = 20;
constexpr int kMarginTop    = 20;
constexpr int kMarginBottom = 40;
constexpr double kHardMin   = -1e5;
constexpr double kHardMax   =  1e5;
constexpr double kMinSpan   = 1e-4;

double clampInRange(double v) {
    return std::clamp(v, kHardMin, kHardMax);
}
}

GraphWidget::GraphWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(400, 400);
    setAutoFillBackground(true);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::WheelFocus);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#101418"));
    setPalette(pal);
}

void GraphWidget::setExpression(AstPtr ast, const QString& sourceText) {
    m_ast = std::move(ast);
    m_sourceText = sourceText;
    m_lastPlotFailed = false;
    update();
}

void GraphWidget::clear() {
    m_ast.reset();
    m_sourceText.clear();
    m_lastPlotFailed = false;
    update();
}

void GraphWidget::setBounds(double xMin, double xMax, double yMin, double yMax) {
    if (xMin < xMax && (xMax - xMin) >= kMinSpan) { m_xMin = xMin; m_xMax = xMax; }
    if (yMin < yMax && (yMax - yMin) >= kMinSpan) { m_yMin = yMin; m_yMax = yMax; }
    update();
}

void GraphWidget::setGridStep(double step) {
    if (step > 0.0) m_gridStep = step;
    update();
}

QPointF GraphWidget::toPixel(double x, double y) const {
    const double w = width()  - kMarginLeft - kMarginRight;
    const double h = height() - kMarginTop  - kMarginBottom;
    const double px = kMarginLeft + (x - m_xMin) / (m_xMax - m_xMin) * w;
    const double py = kMarginTop  + (m_yMax - y) / (m_yMax - m_yMin) * h;
    return {px, py};
}

QPointF GraphWidget::toMath(QPointF pixel) const {
    const double w = width()  - kMarginLeft - kMarginRight;
    const double h = height() - kMarginTop  - kMarginBottom;
    const double mx = m_xMin + (pixel.x() - kMarginLeft) / w * (m_xMax - m_xMin);
    const double my = m_yMax - (pixel.y() - kMarginTop)  / h * (m_yMax - m_yMin);
    return {mx, my};
}

int GraphWidget::autoSampleCount() const {
    // ~2 samples per horizontal pixel of the plotting area gives smooth curves
    // without wasting cycles. Clamped so very small or huge windows behave.
    const int plotW = std::max(1, width() - kMarginLeft - kMarginRight);
    return std::clamp(plotW * 2, 500, 8000);
}

void GraphWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.fillRect(rect(), QColor("#101418"));
    const QRectF plot(kMarginLeft, kMarginTop,
                      width()  - kMarginLeft - kMarginRight,
                      height() - kMarginTop  - kMarginBottom);
    p.fillRect(plot, QColor("#161b22"));

    drawGrid(p);
    drawAxes(p);
    drawCurve(p);
    drawLegend(p);
    drawHover(p);
}

void GraphWidget::drawGrid(QPainter& p) const {
    if (m_gridStep <= 0.0) return;

    // Skip the grid entirely if the chosen step would draw more lines than pixels.
    const double xLines = (m_xMax - m_xMin) / m_gridStep;
    const double yLines = (m_yMax - m_yMin) / m_gridStep;
    if (xLines > 400.0 || yLines > 400.0) return;

    QPen gridPen(QColor(255, 255, 255, 28));
    gridPen.setWidth(1);
    p.setPen(gridPen);

    const double startX = std::ceil(m_xMin / m_gridStep) * m_gridStep;
    for (double x = startX; x <= m_xMax + 1e-9; x += m_gridStep) {
        p.drawLine(toPixel(x, m_yMax), toPixel(x, m_yMin));
    }
    const double startY = std::ceil(m_yMin / m_gridStep) * m_gridStep;
    for (double y = startY; y <= m_yMax + 1e-9; y += m_gridStep) {
        p.drawLine(toPixel(m_xMin, y), toPixel(m_xMax, y));
    }
}

void GraphWidget::drawAxes(QPainter& p) const {
    QPen axisPen(QColor(220, 220, 220));
    axisPen.setWidth(2);
    p.setPen(axisPen);

    const bool yAxisVisible = (m_xMin <= 0.0 && m_xMax >= 0.0);
    const bool xAxisVisible = (m_yMin <= 0.0 && m_yMax >= 0.0);

    if (xAxisVisible) {
        p.drawLine(toPixel(m_xMin, 0.0), toPixel(m_xMax, 0.0));
    }
    if (yAxisVisible) {
        p.drawLine(toPixel(0.0, m_yMax), toPixel(0.0, m_yMin));
    }

    p.setFont(QFont("Menlo", 9));
    QFontMetrics fm(p.font());
    auto labelText = [](double v) {
        if (std::fabs(v) < 1e-9) return QString("0");
        return QString::number(v, 'g', 6);
    };

    p.setPen(QColor(200, 200, 200));
    if (m_gridStep > 0.0) {
        const double xLines = (m_xMax - m_xMin) / m_gridStep;
        const double yLines = (m_yMax - m_yMin) / m_gridStep;
        if (xLines <= 400.0) {
            const double startX = std::ceil(m_xMin / m_gridStep) * m_gridStep;
            const double yForLabel = std::clamp(0.0, m_yMin, m_yMax);
            for (double x = startX; x <= m_xMax + 1e-9; x += m_gridStep) {
                const auto pt = toPixel(x, yForLabel);
                const QString text = labelText(x);
                const int textW = fm.horizontalAdvance(text);
                p.drawLine(QPointF(pt.x(), pt.y() - 4), QPointF(pt.x(), pt.y() + 4));
                p.drawText(QPointF(pt.x() - textW / 2.0, pt.y() + 16), text);
            }
        }
        if (yLines <= 400.0) {
            const double startY = std::ceil(m_yMin / m_gridStep) * m_gridStep;
            const double xForLabel = std::clamp(0.0, m_xMin, m_xMax);
            for (double y = startY; y <= m_yMax + 1e-9; y += m_gridStep) {
                if (std::fabs(y) < 1e-9) continue;
                const auto pt = toPixel(xForLabel, y);
                const QString text = labelText(y);
                const int textW = fm.horizontalAdvance(text);
                p.drawLine(QPointF(pt.x() - 4, pt.y()), QPointF(pt.x() + 4, pt.y()));
                p.drawText(QPointF(pt.x() - textW - 6, pt.y() + fm.ascent() / 2 - 2), text);
            }
        }
    }

    p.setFont(QFont("Menlo", 11, QFont::Bold));
    p.setPen(QColor(240, 240, 240));
    if (xAxisVisible) {
        const auto pt = toPixel(m_xMax, 0.0);
        p.drawText(QPointF(pt.x() - 14, pt.y() - 8), "X");
    }
    if (yAxisVisible) {
        const auto pt = toPixel(0.0, m_yMax);
        p.drawText(QPointF(pt.x() + 6, pt.y() + 12), "Y");
    }
}

void GraphWidget::drawCurve(QPainter& p) const {
    if (!m_ast) return;

    QPen curvePen(QColor("#4aa3ff"));
    curvePen.setWidth(2);
    p.setPen(curvePen);

    const QRectF plot(kMarginLeft, kMarginTop,
                      width()  - kMarginLeft - kMarginRight,
                      height() - kMarginTop  - kMarginBottom);
    p.setClipRect(plot);

    const int n = autoSampleCount();
    QPainterPath path;
    bool penDown = false;
    int drawnSegments = 0;
    double prevY = 0.0;
    bool prevValid = false;

    for (int i = 0; i < n; ++i) {
        const double t = static_cast<double>(i) / (n - 1);
        const double x = m_xMin + t * (m_xMax - m_xMin);
        const double y = Evaluator::evaluate(m_ast.get(), x);

        const bool valid = std::isfinite(y) && y >= m_yMin && y <= m_yMax;
        if (valid) {
            const QPointF pt = toPixel(x, y);
            if (!penDown) {
                path.moveTo(pt);
                penDown = true;
            } else if (prevValid && std::fabs(y - prevY) > (m_yMax - m_yMin) * 0.5) {
                // Probable asymptote — start a new sub-path instead of bridging across.
                path.moveTo(pt);
            } else {
                path.lineTo(pt);
                ++drawnSegments;
            }
            prevY = y;
            prevValid = true;
        } else {
            penDown = false;
            prevValid = false;
        }
    }

    if (drawnSegments > 0) {
        p.drawPath(path);
    }
    p.setClipping(false);

    const_cast<GraphWidget*>(this)->m_lastPlotFailed = (drawnSegments == 0);
}

void GraphWidget::drawLegend(QPainter& p) const {
    if (!m_ast || m_sourceText.isEmpty()) return;
    p.setPen(QColor(220, 220, 220));
    p.setFont(QFont("Menlo", 11));
    const QString text = QStringLiteral("y = ") + m_sourceText;
    p.drawText(QPointF(kMarginLeft + 8, kMarginTop + 16), text);
}

void GraphWidget::drawHover(QPainter& p) const {
    if (!m_hovering) return;
    const QRectF plot(kMarginLeft, kMarginTop,
                      width()  - kMarginLeft - kMarginRight,
                      height() - kMarginTop  - kMarginBottom);
    if (!plot.contains(m_hoverPx)) return;

    const QPointF math = toMath(m_hoverPx);
    QString text = QStringLiteral("x = %1, y = %2")
                       .arg(math.x(), 0, 'g', 6)
                       .arg(math.y(), 0, 'g', 6);
    if (m_ast) {
        const double fy = Evaluator::evaluate(m_ast.get(), math.x());
        if (std::isfinite(fy)) {
            text += QStringLiteral("   f(x) = %1").arg(fy, 0, 'g', 6);
        }
    }

    p.setFont(QFont("Menlo", 10));
    QFontMetrics fm(p.font());
    const int textW = fm.horizontalAdvance(text);
    const int textH = fm.height();
    const QRectF box(plot.right() - textW - 16, plot.top() + 8, textW + 12, textH + 6);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 170));
    p.drawRoundedRect(box, 4, 4);
    p.setPen(QColor(230, 230, 230));
    p.drawText(box, Qt::AlignCenter, text);
}

void GraphWidget::zoomAt(QPointF pixel, double factor) {
    if (factor <= 0.0) return;
    const QPointF math = toMath(pixel);

    double newXMin = math.x() - (math.x() - m_xMin) / factor;
    double newXMax = math.x() + (m_xMax - math.x()) / factor;
    double newYMin = math.y() - (math.y() - m_yMin) / factor;
    double newYMax = math.y() + (m_yMax - math.y()) / factor;

    newXMin = clampInRange(newXMin);
    newXMax = clampInRange(newXMax);
    newYMin = clampInRange(newYMin);
    newYMax = clampInRange(newYMax);

    if ((newXMax - newXMin) < kMinSpan || (newYMax - newYMin) < kMinSpan) return;

    m_xMin = newXMin;
    m_xMax = newXMax;
    m_yMin = newYMin;
    m_yMax = newYMax;

    emit viewChanged(m_xMin, m_xMax, m_yMin, m_yMax);
    update();
}

void GraphWidget::wheelEvent(QWheelEvent* event) {
    const int delta = event->angleDelta().y();
    if (delta == 0) {
        event->ignore();
        return;
    }
    // Each notch (120 units) ≈ 1.2× zoom. Up = zoom in.
    const double notches = delta / 120.0;
    const double factor  = std::pow(1.2, notches);
    zoomAt(event->position(), factor);
    event->accept();
}

void GraphWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_panning = true;
        m_panAnchorPx = event->pos();
        m_panAnchorXMin = m_xMin;
        m_panAnchorYMin = m_yMin;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void GraphWidget::mouseMoveEvent(QMouseEvent* event) {
    m_hovering = true;
    m_hoverPx = event->pos();

    if (m_panning) {
        const double w = std::max(1, width()  - kMarginLeft - kMarginRight);
        const double h = std::max(1, height() - kMarginTop  - kMarginBottom);
        const double dxPx = event->pos().x() - m_panAnchorPx.x();
        const double dyPx = event->pos().y() - m_panAnchorPx.y();
        const double xSpan = m_xMax - m_xMin;
        const double ySpan = m_yMax - m_yMin;
        const double dxMath = -dxPx / w * xSpan;
        const double dyMath =  dyPx / h * ySpan;

        double newXMin = clampInRange(m_panAnchorXMin + dxMath);
        double newYMin = clampInRange(m_panAnchorYMin + dyMath);
        double newXMax = clampInRange(newXMin + xSpan);
        double newYMax = clampInRange(newYMin + ySpan);
        // If clamping shrank the span, reflect that back into min.
        newXMin = newXMax - xSpan;
        newYMin = newYMax - ySpan;

        m_xMin = newXMin;
        m_xMax = newXMax;
        m_yMin = newYMin;
        m_yMax = newYMax;

        emit viewChanged(m_xMin, m_xMax, m_yMin, m_yMax);
    }
    update();
    QWidget::mouseMoveEvent(event);
}

void GraphWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_panning) {
        m_panning = false;
        setCursor(Qt::CrossCursor);
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void GraphWidget::leaveEvent(QEvent* event) {
    m_hovering = false;
    update();
    QWidget::leaveEvent(event);
}
