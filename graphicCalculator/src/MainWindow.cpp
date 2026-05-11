#include "MainWindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "parser/Parser.h"
#include "parser/Tokenizer.h"
#include "widgets/GraphWidget.h"
#include "widgets/InputField.h"
#include "widgets/KeyboardWidget.h"
#include "widgets/SettingsPanel.h"

namespace {
constexpr double kDefaultXMin = -100.0;
constexpr double kDefaultXMax =  100.0;
constexpr double kDefaultYMin = -100.0;
constexpr double kDefaultYMax =  100.0;
constexpr double kDefaultStep =   10.0;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Графический калькулятор функций"));
    setMinimumSize(1100, 760);
    setupUi();

    m_statusBar = statusBar();
    m_cursorLabel = new QLabel(this);
    m_cursorLabel->setStyleSheet("color: #cccccc; padding: 0 8px;");
    m_statusBar->addPermanentWidget(m_cursorLabel);
    setStatus(QStringLiteral("Готов к работе"));
    refreshCursorLabel();
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    m_input = new InputField(this);
    mainLayout->addWidget(m_input);

    auto* contentRow = new QHBoxLayout();
    contentRow->setSpacing(8);

    auto* leftCol = new QVBoxLayout();
    m_keyboard = new KeyboardWidget(this);
    leftCol->addWidget(m_keyboard);

    m_settings = new SettingsPanel(this);
    m_settings->setValues(kDefaultXMin, kDefaultXMax, kDefaultYMin, kDefaultYMax,
                          kDefaultStep);
    leftCol->addWidget(m_settings);
    leftCol->addStretch(1);

    auto* leftWrapper = new QWidget(this);
    leftWrapper->setLayout(leftCol);
    leftWrapper->setMaximumWidth(360);
    contentRow->addWidget(leftWrapper);

    m_graph = new GraphWidget(this);
    m_graph->setBounds(kDefaultXMin, kDefaultXMax, kDefaultYMin, kDefaultYMax);
    m_graph->setGridStep(kDefaultStep);
    contentRow->addWidget(m_graph, /*stretch=*/1);

    mainLayout->addLayout(contentRow, /*stretch=*/1);

    // Wire up signals.
    connect(m_input, &InputField::buildRequested, this, &MainWindow::onBuildRequested);
    connect(m_input, &InputField::clearRequested, this, &MainWindow::onClearRequested);
    connect(m_input, &InputField::cursorPositionChanged, this,
            [this](int, int) { onCursorPositionChanged(); });
    connect(m_input, &InputField::textChanged, this,
            [this](const QString&) { refreshCursorLabel(); });

    connect(m_keyboard, &KeyboardWidget::textRequested,       this, &MainWindow::onTextRequested);
    connect(m_keyboard, &KeyboardWidget::functionRequested,   this, &MainWindow::onFunctionRequested);
    connect(m_keyboard, &KeyboardWidget::cursorMoveRequested, this, &MainWindow::onCursorMoveRequested);
    connect(m_keyboard, &KeyboardWidget::clearRequested,      this, &MainWindow::onClearRequested);
    connect(m_keyboard, &KeyboardWidget::backspaceRequested,  this, &MainWindow::onBackspaceRequested);
    connect(m_keyboard, &KeyboardWidget::buildRequested,      this, &MainWindow::onBuildRequested);

    connect(m_settings, &SettingsPanel::boundsChanged,   this, &MainWindow::onBoundsChanged);
    connect(m_settings, &SettingsPanel::gridStepChanged, this, &MainWindow::onGridStepChanged);
    connect(m_settings, &SettingsPanel::resetRequested,  this, &MainWindow::onResetSettings);

    connect(m_graph, &GraphWidget::viewChanged, this, &MainWindow::onGraphViewChanged);
}

void MainWindow::onTextRequested(const QString& text) {
    m_input->insertAtCursor(text);
}

void MainWindow::onFunctionRequested(const QString& name) {
    m_input->insertFunctionCall(name);
}

void MainWindow::onCursorMoveRequested(int delta) {
    int pos = m_input->cursorPosition() + delta;
    const int maxPos = static_cast<int>(m_input->text().length());
    pos = std::max(0, std::min(pos, maxPos));
    m_input->setCursorPosition(pos);
    m_input->setFocus();
}

void MainWindow::onClearRequested() {
    m_input->clear();
    m_graph->clear();
    setStatus(QStringLiteral("Очищено"));
    refreshCursorLabel();
    m_input->setFocus();
}

void MainWindow::onBackspaceRequested() {
    m_input->backspaceAtCursor();
}

void MainWindow::onBuildRequested() {
    buildGraph();
    m_input->setFocus();
}

void MainWindow::onBoundsChanged(double xMin, double xMax, double yMin, double yMax) {
    if (xMin >= xMax || yMin >= yMax) {
        setStatus(QStringLiteral("Ошибка: некорректный диапазон"), true);
        return;
    }
    m_graph->setBounds(xMin, xMax, yMin, yMax);
    setStatus(QStringLiteral("Диапазон обновлён"));
}

void MainWindow::onGridStepChanged(double step) {
    m_graph->setGridStep(step);
    setStatus(QStringLiteral("Шаг сетки: %1").arg(step));
}

void MainWindow::onGraphViewChanged(double xMin, double xMax, double yMin, double yMax) {
    m_settings->setBoundsQuietly(xMin, xMax, yMin, yMax);
    setStatus(QStringLiteral("Масштаб: X[%1; %2]  Y[%3; %4]")
                  .arg(xMin, 0, 'g', 5).arg(xMax, 0, 'g', 5)
                  .arg(yMin, 0, 'g', 5).arg(yMax, 0, 'g', 5));
}

void MainWindow::onResetSettings() {
    m_settings->setValues(kDefaultXMin, kDefaultXMax, kDefaultYMin, kDefaultYMax,
                          kDefaultStep);
    m_graph->setBounds(kDefaultXMin, kDefaultXMax, kDefaultYMin, kDefaultYMax);
    m_graph->setGridStep(kDefaultStep);
    setStatus(QStringLiteral("Параметры сброшены"));
}

void MainWindow::onCursorPositionChanged() {
    refreshCursorLabel();
}

void MainWindow::refreshCursorLabel() {
    if (!m_cursorLabel) return;
    m_cursorLabel->setText(
        QStringLiteral("Курсор: %1 / %2")
            .arg(m_input->cursorPosition())
            .arg(m_input->text().length()));
}

void MainWindow::buildGraph() {
    const QString expression = m_input->text().trimmed();
    if (expression.isEmpty()) {
        m_graph->clear();
        setStatus(QStringLiteral("Ошибка: Введите функцию"), true);
        return;
    }

    Tokenizer tokenizer;
    auto tokens = tokenizer.tokenize(expression.toStdString());
    if (tokenizer.hasError()) {
        m_graph->clear();
        setStatus(QStringLiteral("Ошибка: ") + QString::fromStdString(tokenizer.error()), true);
        return;
    }

    Parser parser;
    auto ast = parser.parse(tokens);
    if (!ast || parser.hasError()) {
        m_graph->clear();
        setStatus(QStringLiteral("Ошибка: Некорректное выражение"), true);
        return;
    }

    m_graph->setExpression(std::move(ast), expression);
    if (m_graph->lastPlotFailed()) {
        setStatus(QStringLiteral("Ошибка: Не удалось построить график"), true);
    } else {
        setStatus(QStringLiteral("График построен"));
    }
}

void MainWindow::setStatus(const QString& msg, bool error) {
    if (!m_statusBar) return;
    m_statusBar->setStyleSheet(error
        ? "QStatusBar { background: #5a3030; color: white; }"
        : "QStatusBar { background: #2a4a2a; color: white; }");
    m_statusBar->showMessage(msg);
}
