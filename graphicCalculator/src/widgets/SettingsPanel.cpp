#include "SettingsPanel.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
constexpr double kHardMin = -1e5;
constexpr double kHardMax =  1e5;
}

SettingsPanel::SettingsPanel(QWidget* parent) : QWidget(parent) {
    auto makeDoubleSpin = [this](double minV, double maxV, double step, double val) {
        auto* s = new QDoubleSpinBox(this);
        s->setRange(minV, maxV);
        s->setSingleStep(step);
        s->setDecimals(4);
        s->setValue(val);
        s->setFocusPolicy(Qt::StrongFocus);
        return s;
    };

    m_xMin     = makeDoubleSpin(kHardMin, kHardMax, 1.0, -100.0);
    m_xMax     = makeDoubleSpin(kHardMin, kHardMax, 1.0,  100.0);
    m_yMin     = makeDoubleSpin(kHardMin, kHardMax, 1.0, -100.0);
    m_yMax     = makeDoubleSpin(kHardMin, kHardMax, 1.0,  100.0);
    m_gridStep = makeDoubleSpin(0.0001,  kHardMax, 1.0,   10.0);

    m_resetBtn = new QPushButton(QStringLiteral("Сброс"), this);
    m_resetBtn->setFocusPolicy(Qt::NoFocus);

    auto* group = new QGroupBox(QStringLiteral("Параметры графика"), this);
    auto* form = new QFormLayout(group);
    form->setLabelAlignment(Qt::AlignRight);

    auto* xRow = new QHBoxLayout();
    xRow->addWidget(m_xMin);
    xRow->addWidget(new QLabel(QStringLiteral("…"), this));
    xRow->addWidget(m_xMax);
    form->addRow(QStringLiteral("Диапазон X:"), xRow);

    auto* yRow = new QHBoxLayout();
    yRow->addWidget(m_yMin);
    yRow->addWidget(new QLabel(QStringLiteral("…"), this));
    yRow->addWidget(m_yMax);
    form->addRow(QStringLiteral("Диапазон Y:"), yRow);

    form->addRow(QStringLiteral("Шаг сетки:"), m_gridStep);
    form->addRow(QString(), m_resetBtn);

    auto* hint = new QLabel(
        QStringLiteral("Колесо мыши — масштаб, ЛКМ-перетаскивание — панорама."),
        this);
    hint->setStyleSheet("color: #aaaaaa; font-size: 11px;");
    hint->setWordWrap(true);
    form->addRow(QString(), hint);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(group);

    auto emitBoundsFn = [this]() { emitBounds(); };
    connect(m_xMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, emitBoundsFn);
    connect(m_xMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, emitBoundsFn);
    connect(m_yMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, emitBoundsFn);
    connect(m_yMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, emitBoundsFn);

    connect(m_gridStep, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) { emit gridStepChanged(v); });
    connect(m_resetBtn, &QPushButton::clicked, this, &SettingsPanel::resetRequested);
}

void SettingsPanel::setValues(double xMin, double xMax, double yMin, double yMax,
                              double gridStep) {
    QSignalBlocker b1(m_xMin), b2(m_xMax), b3(m_yMin), b4(m_yMax), b5(m_gridStep);
    m_xMin->setValue(xMin);
    m_xMax->setValue(xMax);
    m_yMin->setValue(yMin);
    m_yMax->setValue(yMax);
    m_gridStep->setValue(gridStep);
}

void SettingsPanel::setBoundsQuietly(double xMin, double xMax, double yMin, double yMax) {
    QSignalBlocker b1(m_xMin), b2(m_xMax), b3(m_yMin), b4(m_yMax);
    m_xMin->setValue(xMin);
    m_xMax->setValue(xMax);
    m_yMin->setValue(yMin);
    m_yMax->setValue(yMax);
}

void SettingsPanel::emitBounds() {
    emit boundsChanged(m_xMin->value(), m_xMax->value(),
                       m_yMin->value(), m_yMax->value());
}
