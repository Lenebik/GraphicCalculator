#pragma once

#include <QMainWindow>
#include <QString>

class InputField;
class KeyboardWidget;
class GraphWidget;
class SettingsPanel;
class QLabel;
class QStatusBar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onTextRequested(const QString& text);
    void onFunctionRequested(const QString& name);
    void onCursorMoveRequested(int delta);
    void onClearRequested();
    void onBackspaceRequested();
    void onBuildRequested();

    void onBoundsChanged(double xMin, double xMax, double yMin, double yMax);
    void onGridStepChanged(double step);
    void onGraphViewChanged(double xMin, double xMax, double yMin, double yMax);
    void onResetSettings();

    void onCursorPositionChanged();

private:
    void setupUi();
    void buildGraph();
    void setStatus(const QString& msg, bool error = false);
    void refreshCursorLabel();

    InputField*     m_input    = nullptr;
    KeyboardWidget* m_keyboard = nullptr;
    GraphWidget*    m_graph    = nullptr;
    SettingsPanel*  m_settings = nullptr;
    QStatusBar*     m_statusBar = nullptr;
    QLabel*         m_cursorLabel = nullptr;
};
