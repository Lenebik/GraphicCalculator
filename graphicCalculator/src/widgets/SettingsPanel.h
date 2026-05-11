#pragma once

#include <QWidget>

class QDoubleSpinBox;
class QPushButton;

class SettingsPanel : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPanel(QWidget* parent = nullptr);

    void setValues(double xMin, double xMax, double yMin, double yMax, double gridStep);
    // Sync only the bounds (e.g. after a mouse zoom/pan) without firing signals.
    void setBoundsQuietly(double xMin, double xMax, double yMin, double yMax);

signals:
    void boundsChanged(double xMin, double xMax, double yMin, double yMax);
    void gridStepChanged(double step);
    void resetRequested();

private:
    void emitBounds();

    QDoubleSpinBox* m_xMin;
    QDoubleSpinBox* m_xMax;
    QDoubleSpinBox* m_yMin;
    QDoubleSpinBox* m_yMax;
    QDoubleSpinBox* m_gridStep;
    QPushButton*    m_resetBtn;
};
