#pragma once

#include <QPoint>
#include <QString>
#include <QWidget>
#include <memory>
#include <vector>

#include "parser/AstNode.h"

class GraphWidget : public QWidget {
    Q_OBJECT
public:
    explicit GraphWidget(QWidget* parent = nullptr);

    // Take ownership of an AST and re-plot.
    void setExpression(AstPtr ast, const QString& sourceText);
    void clear();

    // Plot bounds in math coordinates (user-adjustable).
    void setBounds(double xMin, double xMax, double yMin, double yMax);
    void setGridStep(double step);

    double xMin() const { return m_xMin; }
    double xMax() const { return m_xMax; }
    double yMin() const { return m_yMin; }
    double yMax() const { return m_yMax; }
    double gridStep() const { return m_gridStep; }

    // True if the last setExpression resulted in zero plottable points.
    bool lastPlotFailed() const { return m_lastPlotFailed; }

signals:
    // Emitted when the user changes the visible region via mouse (zoom/pan).
    void viewChanged(double xMin, double xMax, double yMin, double yMax);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QPointF toPixel(double x, double y) const;
    QPointF toMath(QPointF pixel) const;
    int autoSampleCount() const;
    void drawGrid(class QPainter& p) const;
    void drawAxes(class QPainter& p) const;
    void drawCurve(class QPainter& p) const;
    void drawLegend(class QPainter& p) const;
    void drawHover(class QPainter& p) const;
    void zoomAt(QPointF pixel, double factor);

    AstPtr m_ast;
    QString m_sourceText;

    double m_xMin = -100.0;
    double m_xMax =  100.0;
    double m_yMin = -100.0;
    double m_yMax =  100.0;
    double m_gridStep = 10.0;

    bool   m_lastPlotFailed = false;
    bool   m_panning = false;
    QPoint m_panAnchorPx;
    double m_panAnchorXMin = 0.0;
    double m_panAnchorYMin = 0.0;
    bool   m_hovering = false;
    QPoint m_hoverPx;
};
